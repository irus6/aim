/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
 * im-aim.c
 * This file is part of AIM.
 *
 * Copyright (C) 2015 Hodong Kim <hodong@cogno.org>
 *
 * AIM is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * AIM is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program;  If not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"
#include <gtk/gtk.h>
#include <gtk/gtkimmodule.h>
#include <glib/gi18n.h>
#include <aim.h>
#include <string.h>
#include <gdk/gdkkeysyms.h>

#define AIM_GTK_TYPE_IM_CONTEXT  (aim_gtk_im_context_get_type ())
#define AIM_GTK_IM_CONTEXT(obj)  (G_TYPE_CHECK_INSTANCE_CAST ((obj), AIM_GTK_TYPE_IM_CONTEXT, AimGtkIMContext))

typedef struct _AimGtkIMContext      AimGtkIMContext;
typedef struct _AimGtkIMContextClass AimGtkIMContextClass;

struct _AimGtkIMContext
{
  GtkIMContext  parent_instance;

  AimIM        *im;
  GdkWindow    *client_window;
  GdkRectangle  cursor_area;
  GSettings    *settings;
  gboolean      is_reset_on_gdk_button_press_event;
  gboolean      is_hook_gdk_event_key;
  gboolean      has_focus;
  gboolean      has_event_filter;
};

struct _AimGtkIMContextClass
{
  GtkIMContextClass parent_class;
};

G_DEFINE_DYNAMIC_TYPE (AimGtkIMContext, aim_gtk_im_context, GTK_TYPE_IM_CONTEXT);

static AimEvent *
translate_gdk_event_key (GdkEventKey *event)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  AimEventType type;

  if (event->type == GDK_KEY_PRESS)
    type = AIM_EVENT_KEY_PRESS;
  else
    type = AIM_EVENT_KEY_RELEASE;

  AimEvent *aim_event = aim_event_new (type);
  aim_event->key.state = event->state;
  aim_event->key.keyval = event->keyval;
  aim_event->key.hardware_keycode = event->hardware_keycode;

  return aim_event;
}

static AimEvent *
translate_xkey_event (XEvent *xevent)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  AimEventType type = AIM_EVENT_NOTHING;

  if (xevent->type == KeyPress)
    type = AIM_EVENT_KEY_PRESS;
  else
    type = AIM_EVENT_KEY_RELEASE;

  AimEvent *aim_event = aim_event_new (type);
  aim_event->key.state  = xevent->xkey.state;
  aim_event->key.keyval = XLookupKeysym (&xevent->xkey,
                            (!(xevent->xkey.state & ShiftMask) !=
                             !(xevent->xkey.state & LockMask)) ? 1 : 0);
  aim_event->key.hardware_keycode = xevent->xkey.keycode;

  return aim_event;
}

static gboolean
aim_gtk_im_context_filter_keypress (GtkIMContext *context,
                                    GdkEventKey  *event)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  gboolean retval = FALSE;
  AimEvent *aim_event = translate_gdk_event_key (event);

  retval = aim_im_filter_event (AIM_GTK_IM_CONTEXT (context)->im, aim_event);
  aim_event_free (aim_event);

  return retval;
}

static void
aim_gtk_im_context_reset (GtkIMContext *context)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  aim_im_reset (AIM_GTK_IM_CONTEXT (context)->im);
}

static GdkFilterReturn
on_gdk_x_event (XEvent          *xevent,
                GdkEvent        *event,
                AimGtkIMContext *context)
{
  g_debug (G_STRLOC ": %s: %p, %" G_GINT64_FORMAT, G_STRFUNC, context,
           g_get_real_time ());

  gboolean retval = FALSE;

  if (context->has_focus == FALSE || context->client_window == NULL)
    return GDK_FILTER_CONTINUE;

  switch (xevent->type)
  {
    case KeyPress:
    case KeyRelease:
      if (context->is_hook_gdk_event_key)
      {
        AimEvent *d_event = translate_xkey_event (xevent);
        retval = aim_im_filter_event (context->im, d_event);
        aim_event_free (d_event);
      }
      break;
    case ButtonPress:
      if (context->is_reset_on_gdk_button_press_event)
        aim_im_reset (context->im);
      break;
    default:
      break;
  }

  if (retval == FALSE)
    return GDK_FILTER_CONTINUE;
  else
    return GDK_FILTER_REMOVE;
}

static void
aim_gtk_im_context_set_client_window (GtkIMContext *context,
                                      GdkWindow    *window)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  AimGtkIMContext *a_context = AIM_GTK_IM_CONTEXT (context);

  if (a_context->client_window)
  {
    g_object_unref (a_context->client_window);
    a_context->client_window = NULL;
  }

  if (window)
    a_context->client_window = g_object_ref (window);
}

static void
aim_gtk_im_context_get_preedit_string (GtkIMContext   *context,
                                       gchar         **str,
                                       PangoAttrList **attrs,
                                       gint           *cursor_pos)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  PangoAttribute *attr;

  aim_im_get_preedit_string (AIM_GTK_IM_CONTEXT (context)->im,
                             str, cursor_pos);

  if (attrs)
  {
    *attrs = pango_attr_list_new ();

    attr = pango_attr_underline_new (PANGO_UNDERLINE_SINGLE);

    if (str)
    {
      attr->start_index = 0;
      attr->end_index   = strlen (*str);
    }

    pango_attr_list_change (*attrs, attr);
  }
}

static void
aim_gtk_im_context_focus_in (GtkIMContext *context)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  AimGtkIMContext *a_context = AIM_GTK_IM_CONTEXT (context);
  a_context->has_focus = TRUE;
  aim_im_focus_in (a_context->im);
}

static void
aim_gtk_im_context_focus_out (GtkIMContext *context)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  AimGtkIMContext *a_context = AIM_GTK_IM_CONTEXT (context);
  aim_im_focus_out (a_context->im);
  a_context->has_focus = FALSE;
}

static void
aim_gtk_im_context_set_cursor_location (GtkIMContext *context,
                                        GdkRectangle *area)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  AimGtkIMContext *aim_context = AIM_GTK_IM_CONTEXT (context);

  if (memcmp (&aim_context->cursor_area, area, sizeof (GdkRectangle)) == 0)
    return;

  aim_context->cursor_area = *area;
  GdkRectangle root_area = *area;

  if (aim_context->client_window)
  {
    gdk_window_get_root_coords (aim_context->client_window,
                                area->x,
                                area->y,
                                &root_area.x,
                                &root_area.y);

    aim_im_set_cursor_location (AIM_GTK_IM_CONTEXT (context)->im,
                                (const AimRectangle *) &root_area);
  }
}

static void
aim_gtk_im_context_set_use_preedit (GtkIMContext *context,
                                    gboolean      use_preedit)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  aim_im_set_use_preedit (AIM_GTK_IM_CONTEXT (context)->im, use_preedit);
}

static gboolean
aim_gtk_im_context_get_surrounding (GtkIMContext  *context,
                                    gchar        **text,
                                    gint          *cursor_index)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return aim_im_get_surrounding (AIM_GTK_IM_CONTEXT (context)->im,
                                 text, cursor_index);
}

static void
aim_gtk_im_context_set_surrounding (GtkIMContext *context,
                                    const char   *text,
                                    gint          len,
                                    gint          cursor_index)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  aim_im_set_surrounding (AIM_GTK_IM_CONTEXT (context)->im,
                          text, len, cursor_index);
}

GtkIMContext *
aim_gtk_im_context_new (void)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return g_object_new (AIM_GTK_TYPE_IM_CONTEXT, NULL);
}

static void
on_commit (AimIM           *im,
           const gchar     *text,
           AimGtkIMContext *context)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_signal_emit_by_name (context, "commit", text);
}

static gboolean
on_delete_surrounding (AimIM           *im,
                       gint             offset,
                       gint             n_chars,
                       AimGtkIMContext *context)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  gboolean retval;
  g_signal_emit_by_name (context,
                         "delete-surrounding", offset, n_chars, &retval);
  return retval;
}

static void
on_preedit_changed (AimIM           *im,
                    AimGtkIMContext *context)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
  g_signal_emit_by_name (context, "preedit-changed");
}

static void
on_preedit_end (AimIM           *im,
                AimGtkIMContext *context)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
  g_signal_emit_by_name (context, "preedit-end");
}

static void
on_preedit_start (AimIM           *im,
                  AimGtkIMContext *context)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
  g_signal_emit_by_name (context, "preedit-start");
}

static gboolean
on_retrieve_surrounding (AimIM           *im,
                         AimGtkIMContext *context)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  gboolean retval;
  g_signal_emit_by_name (context, "retrieve-surrounding", &retval);

  return retval;
}

static void
aim_gtk_im_context_update_event_filter (AimGtkIMContext *context)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (context->is_reset_on_gdk_button_press_event ||
      context->is_hook_gdk_event_key)
  {
    if (context->has_event_filter == FALSE)
    {
      context->has_event_filter = TRUE;
      gdk_window_add_filter (NULL, (GdkFilterFunc) on_gdk_x_event, context);
    }
  }
  else
  {
    if (context->has_event_filter == TRUE)
    {
      context->has_event_filter = FALSE;
      gdk_window_remove_filter (NULL, (GdkFilterFunc) on_gdk_x_event, context);
    }
  }
}

static void
on_changed_reset_on_gdk_button_press_event (GSettings       *settings,
                                            gchar           *key,
                                            AimGtkIMContext *context)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  context->is_reset_on_gdk_button_press_event =
    g_settings_get_boolean (context->settings, key);

  aim_gtk_im_context_update_event_filter (context);
}

static void
on_changed_hook_gdk_event_key (GSettings       *settings,
                               gchar           *key,
                               AimGtkIMContext *context)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  context->is_hook_gdk_event_key =
    g_settings_get_boolean (context->settings, key);

  aim_gtk_im_context_update_event_filter (context);
}

static void
aim_gtk_im_context_init (AimGtkIMContext *context)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  context->im = aim_im_new ();

  g_signal_connect (context->im, "commit",
                    G_CALLBACK (on_commit), context);
  g_signal_connect (context->im, "delete-surrounding",
                    G_CALLBACK (on_delete_surrounding), context);
  g_signal_connect (context->im, "preedit-changed",
                    G_CALLBACK (on_preedit_changed), context);
  g_signal_connect (context->im, "preedit-end",
                    G_CALLBACK (on_preedit_end), context);
  g_signal_connect (context->im, "preedit-start",
                    G_CALLBACK (on_preedit_start), context);
  g_signal_connect (context->im, "retrieve-surrounding",
                    G_CALLBACK (on_retrieve_surrounding), context);

  context->settings = g_settings_new ("org.aim.clients.gtk");

  context->is_reset_on_gdk_button_press_event =
    g_settings_get_boolean (context->settings,
                            "reset-on-gdk-button-press-event");

  context->is_hook_gdk_event_key =
    g_settings_get_boolean (context->settings, "hook-gdk-event-key");

  aim_gtk_im_context_update_event_filter (context);

  g_signal_connect (context->settings,
                    "changed::reset-on-gdk-button-press-event",
                    G_CALLBACK (on_changed_reset_on_gdk_button_press_event),
                    context);
  g_signal_connect (context->settings, "changed::hook-gdk-event-key",
                    G_CALLBACK (on_changed_hook_gdk_event_key), context);
}

static void
aim_gtk_im_context_finalize (GObject *object)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  AimGtkIMContext *context = AIM_GTK_IM_CONTEXT (object);

  if (context->has_event_filter)
    gdk_window_remove_filter (NULL, (GdkFilterFunc) on_gdk_x_event, context);

  g_object_unref (context->im);
  g_object_unref (context->settings);

  if (context->client_window)
    g_object_unref (context->client_window);

  G_OBJECT_CLASS (aim_gtk_im_context_parent_class)->finalize (object);
}

static void
aim_gtk_im_context_class_init (AimGtkIMContextClass *class)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GObjectClass *object_class = G_OBJECT_CLASS (class);
  GtkIMContextClass *im_context_class = GTK_IM_CONTEXT_CLASS (class);

  im_context_class->set_client_window   = aim_gtk_im_context_set_client_window;
  im_context_class->get_preedit_string  = aim_gtk_im_context_get_preedit_string;
  im_context_class->filter_keypress     = aim_gtk_im_context_filter_keypress;
  im_context_class->focus_in            = aim_gtk_im_context_focus_in;
  im_context_class->focus_out           = aim_gtk_im_context_focus_out;
  im_context_class->reset               = aim_gtk_im_context_reset;
  im_context_class->set_cursor_location = aim_gtk_im_context_set_cursor_location;
  im_context_class->set_use_preedit     = aim_gtk_im_context_set_use_preedit;
  im_context_class->set_surrounding     = aim_gtk_im_context_set_surrounding;
  im_context_class->get_surrounding     = aim_gtk_im_context_get_surrounding;

  object_class->finalize = aim_gtk_im_context_finalize;
}

static void
aim_gtk_im_context_class_finalize (AimGtkIMContextClass *class)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
}

static const GtkIMContextInfo aim_info = {
  PACKAGE,          /* ID */
  N_("AIM"),        /* Human readable name */
  GETTEXT_PACKAGE,  /* Translation domain */
  AIM_LOCALE_DIR,   /* Directory for bindtextdomain */
  "ko:ja:zh"        /* Languages for which this module is the default */
};

static const GtkIMContextInfo *info_list[] = {
  &aim_info
};

G_MODULE_EXPORT void im_module_init (GTypeModule *type_module)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  aim_gtk_im_context_register_type (type_module);
}

G_MODULE_EXPORT void im_module_exit (void)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
}

G_MODULE_EXPORT void im_module_list (const GtkIMContextInfo ***contexts,
                                     int                      *n_contexts)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  *contexts = info_list;
  *n_contexts = G_N_ELEMENTS (info_list);
}

G_MODULE_EXPORT GtkIMContext *im_module_create (const gchar *context_id)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (g_strcmp0 (context_id, PACKAGE) == 0)
    return aim_gtk_im_context_new ();
  else
    return NULL;
}
