/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
 * aim-connection.c
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

#include "aim-connection.h"
#include "aim-events.h"
#include "aim-marshalers.h"
#include "aim-private.h"
#include <string.h>
#include <X11/Xutil.h>
#include "IMdkit/Xi18n.h"

enum {
  ENGINE_CHANGED,
  LAST_SIGNAL
};

static guint connection_signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE (AimConnection, aim_connection, G_TYPE_OBJECT);

static void
aim_connection_init (AimConnection *connection)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  connection->result = g_slice_new0 (AimResult);
}

static void
aim_connection_finalize (GObject *object)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  AimConnection *connection = AIM_CONNECTION (object);
  aim_message_unref (connection->result->reply);

  if (connection->source)
  {
    g_source_destroy (connection->source);
    g_source_unref   (connection->source);
  }

  if (connection->socket_connection)
    g_object_unref (connection->socket_connection);

  g_slice_free (AimResult, connection->result);

  G_OBJECT_CLASS (aim_connection_parent_class)->finalize (object);
}

static void
aim_connection_class_init (AimConnectionClass *class)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GObjectClass *object_class = G_OBJECT_CLASS (class);
  object_class->finalize = aim_connection_finalize;

  connection_signals[ENGINE_CHANGED] =
    g_signal_new (g_intern_static_string ("engine-changed"),
                  G_TYPE_FROM_CLASS (class),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (AimConnectionClass, engine_changed),
                  NULL, NULL,
                  aim_cclosure_marshal_VOID__STRING,
                  G_TYPE_NONE, 1,
                  G_TYPE_STRING);
}

void
on_signal_engine_changed (AimConnection *connection,
                          const gchar   *name,
                          gpointer       user_data)
{
  g_debug (G_STRLOC ": %s: %s: connection id = %d", G_STRFUNC,
           name, connection->id);

  GList *l = connection->server->agents_list;
  while (l != NULL)
  {
    GList *next = l->next;
    aim_send_message (AIM_CONNECTION (l->data)->socket,
                      AIM_MESSAGE_ENGINE_CHANGED,
                      (gchar *) name, strlen (name) + 1, NULL);
    l = next;
  }
}

AimConnection *
aim_connection_new (AimConnectionType  type,
                    AimEngine         *engine,
                    gpointer           cb_user_data)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  AimConnection *connection = g_object_new (AIM_TYPE_CONNECTION, NULL);

  connection->type            = type;
  connection->engine          = engine;
  connection->use_preedit     = TRUE;
  connection->preedit_state   = AIM_PREEDIT_STATE_END;
  connection->is_english_mode = TRUE;
  connection->cb_user_data    = cb_user_data;

  g_signal_connect (connection,
                    "engine-changed",
                    G_CALLBACK (on_signal_engine_changed),
                    NULL);
  return connection;
}

guint16
aim_connection_get_id (AimConnection *connection)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_val_if_fail (AIM_IS_CONNECTION (connection), 0);

  return connection->id;
}

void aim_connection_reset (AimConnection *connection)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  aim_engine_reset (connection->engine, connection);
}

void aim_connection_focus_in (AimConnection *connection)
{
  g_debug (G_STRLOC ": %s: connection id = %d", G_STRFUNC, connection->id);

  aim_engine_focus_in (connection->engine);

  g_signal_emit_by_name (connection, "engine-changed",
                         aim_engine_get_name (connection->engine));
}

void aim_connection_focus_out (AimConnection *connection)
{
  g_debug (G_STRLOC ": %s: connection id = %d", G_STRFUNC, connection->id);

  aim_engine_focus_out (connection->engine, connection);

  g_signal_emit_by_name (connection, "engine-changed", "focus-out");
}

void aim_connection_set_next_engine (AimConnection *connection)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  connection->engine = aim_server_get_next_instance (connection->server,
                                                     connection->engine);
  g_signal_emit_by_name (connection, "engine-changed",
                         aim_engine_get_name (connection->engine));
}

gboolean aim_connection_filter_event (AimConnection *connection,
                                      AimEvent      *event)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (aim_event_matches (event,
                         (const AimKey **) connection->server->hotkeys))
  {
    if (event->key.type == AIM_EVENT_KEY_RELEASE)
    {
      aim_connection_reset (connection);
      aim_connection_set_next_engine (connection);
    }

    return TRUE;
  }

  return aim_engine_filter_event (connection->engine, connection, event);
}

void
aim_connection_get_preedit_string (AimConnection  *connection,
                                   gchar         **str,
                                   gint           *cursor_pos)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (G_LIKELY (connection->use_preedit == TRUE))
    aim_engine_get_preedit_string (connection->engine, str, cursor_pos);
  else
  {
    if (str)
      *str = g_strdup ("");

    if (cursor_pos)
      *cursor_pos = 0;
  }
}

void
aim_connection_set_surrounding (AimConnection *connection,
                                const char    *text,
                                gint           len,
                                gint           cursor_index)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  aim_engine_set_surrounding (connection->engine, text, len, cursor_index);
}

gboolean
aim_connection_get_surrounding (AimConnection  *connection,
                                gchar         **text,
                                gint           *cursor_index)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return aim_engine_get_surrounding (connection->engine, connection,
                                     text, cursor_index);
}

void
aim_connection_set_cursor_location (AimConnection      *connection,
                                    const AimRectangle *area)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  connection->cursor_area = *area;
  aim_engine_set_cursor_location (connection->engine, area);
}

void
aim_connection_set_use_preedit (AimConnection *connection,
                                gboolean       use_preedit)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (connection->use_preedit == TRUE && use_preedit == FALSE)
  {
    connection->use_preedit = FALSE;

    if (connection->preedit_state == AIM_PREEDIT_STATE_START)
    {
      gchar *preedit_string;
      gint   cursor_pos;
      aim_connection_get_preedit_string   (connection,
                                           &preedit_string,
                                           &cursor_pos);
      aim_connection_emit_preedit_changed (connection,
                                           preedit_string,
                                           cursor_pos);
      aim_connection_emit_preedit_end     (connection);
      g_free (preedit_string);
    }
  }
  else if (connection->use_preedit == FALSE && use_preedit == TRUE)
  {
    gchar *preedit_string;
    gint   cursor_pos;

    connection->use_preedit = TRUE;

    aim_connection_get_preedit_string (connection,
                                       &preedit_string,
                                       &cursor_pos);
    if (preedit_string[0] != 0)
    {
      aim_connection_emit_preedit_start   (connection);
      aim_connection_emit_preedit_changed (connection,
                                           preedit_string,
                                           cursor_pos);
    }

    g_free (preedit_string);
  }
}

void
aim_connection_emit_preedit_start (AimConnection *connection)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  switch (connection->type)
  {
    case AIM_CONNECTION_AIM_IM:
      if (G_UNLIKELY (connection->use_preedit == FALSE &&
                      connection->preedit_state == AIM_PREEDIT_STATE_END))
        return;

      aim_send_message (connection->socket, AIM_MESSAGE_PREEDIT_START,
                        NULL, 0, NULL);
      aim_result_iteration_until (connection->result, NULL,
                                  AIM_MESSAGE_PREEDIT_START_REPLY);
      connection->preedit_state = AIM_PREEDIT_STATE_START;
      break;
    case AIM_CONNECTION_XIM:
      {
        XIMS xims = connection->cb_user_data;
        IMPreeditStateStruct preedit_state_data = {0};
        preedit_state_data.connect_id = connection->xim_connect_id;
        preedit_state_data.icid       = connection->id;
        IMPreeditStart (xims, (XPointer) &preedit_state_data);

        IMPreeditCBStruct preedit_cb_data = {0};
        preedit_cb_data.major_code = XIM_PREEDIT_START;
        preedit_cb_data.connect_id = connection->xim_connect_id;
        preedit_cb_data.icid       = connection->id;
        IMCallCallback (xims, (XPointer) &preedit_cb_data);
      }
      break;
    default:
      g_warning ("Unknown type: %d", connection->type);
      break;
  }
}

void
aim_connection_emit_preedit_changed (AimConnection *connection,
                                     const gchar   *preedit_string,
                                     gint           cursor_pos)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  switch (connection->type)
  {
    case AIM_CONNECTION_AIM_IM:
      if (G_UNLIKELY (connection->use_preedit == FALSE &&
                      connection->preedit_state == AIM_PREEDIT_STATE_END))
        return;

      {
        gsize data_len = strlen (preedit_string) + 1 + sizeof (gint);
        gchar *data = g_strndup (preedit_string, data_len - 1);
        *(gint *) (data + data_len - sizeof (gint)) = cursor_pos;

        aim_send_message (connection->socket, AIM_MESSAGE_PREEDIT_CHANGED,
                          data, data_len, g_free);
        aim_result_iteration_until (connection->result, NULL,
                                    AIM_MESSAGE_PREEDIT_CHANGED_REPLY);
      }
      break;
    case AIM_CONNECTION_XIM:
      {
        XIMS xims = connection->cb_user_data;
        gchar *preedit_string;
        gint   cursor_pos;
        aim_connection_get_preedit_string (connection, &preedit_string,
                                           &cursor_pos);
        IMPreeditCBStruct preedit_cb_data = {0};
        XIMText           text;
        XTextProperty     text_property;

        static XIMFeedback *feedback;
        gint i, len;

        if (preedit_string == NULL)
          return;

        len = g_utf8_strlen (preedit_string, -1);

        feedback = g_malloc (sizeof (XIMFeedback) * (len + 1));

        for (i = 0; i < len; i++)
          feedback[i] = XIMUnderline;

        feedback[len] = 0;

        preedit_cb_data.major_code = XIM_PREEDIT_DRAW;
        preedit_cb_data.connect_id = connection->xim_connect_id;
        preedit_cb_data.icid = connection->id;
        preedit_cb_data.todo.draw.caret = len;
        preedit_cb_data.todo.draw.chg_first = 0;
        preedit_cb_data.todo.draw.chg_length = connection->xim_preedit_length;
        preedit_cb_data.todo.draw.text = &text;

        text.feedback = feedback;

        if (len > 0)
        {
          Xutf8TextListToTextProperty (xims->core.display,
                                       (char **) &preedit_string, 1,
                                       XCompoundTextStyle, &text_property);
          text.encoding_is_wchar = 0;
          text.length = strlen ((char *) text_property.value);
          text.string.multi_byte = (char *) text_property.value;
          IMCallCallback (xims, (XPointer) &preedit_cb_data);
          XFree (text_property.value);
        }
        else
        {
          text.encoding_is_wchar = 0;
          text.length = 0;
          text.string.multi_byte = "";
          IMCallCallback (xims, (XPointer) &preedit_cb_data);
          len = 0;
        }

        connection->xim_preedit_length = len;
        g_free (feedback);
      }
      break;
    default:
      g_warning ("Unknown type: %d", connection->type);
      break;
  }
}

void
aim_connection_emit_preedit_end (AimConnection *connection)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  switch (connection->type)
  {
    case AIM_CONNECTION_AIM_IM:
      if (G_UNLIKELY (connection->use_preedit == FALSE &&
                      connection->preedit_state == AIM_PREEDIT_STATE_END))
        return;

      aim_send_message (connection->socket, AIM_MESSAGE_PREEDIT_END,
                        NULL, 0, NULL);
      aim_result_iteration_until (connection->result, NULL,
                                  AIM_MESSAGE_PREEDIT_END_REPLY);
      connection->preedit_state = AIM_PREEDIT_STATE_END;
      break;
    case AIM_CONNECTION_XIM:
      {
        XIMS xims = connection->cb_user_data;
        IMPreeditStateStruct preedit_state_data = {0};
        preedit_state_data.connect_id = connection->xim_connect_id;
        preedit_state_data.icid       = connection->id;
        IMPreeditEnd (xims, (XPointer) &preedit_state_data);

        IMPreeditCBStruct preedit_cb_data = {0};
        preedit_cb_data.major_code = XIM_PREEDIT_DONE;
        preedit_cb_data.connect_id = connection->xim_connect_id;
        preedit_cb_data.icid       = connection->id;
        IMCallCallback (xims, (XPointer) &preedit_cb_data);
      }
      break;
    default:
      g_warning ("Unknown type: %d", connection->type);
      break;
  }
}

void
aim_connection_emit_commit (AimConnection *connection,
                            const gchar   *text)
{
  g_debug (G_STRLOC ": %s: id = %d", G_STRFUNC, connection->id);

  switch (connection->type)
  {
    case AIM_CONNECTION_AIM_IM:
      aim_send_message (connection->socket, AIM_MESSAGE_COMMIT,
                        (gchar *) text, strlen (text) + 1, NULL);
      aim_result_iteration_until (connection->result, NULL,
                                  AIM_MESSAGE_COMMIT_REPLY);
      break;
    case AIM_CONNECTION_XIM:
      {
        XIMS xims = connection->cb_user_data;
        XTextProperty property;
        Xutf8TextListToTextProperty (xims->core.display,
                                     (char **)&text, 1, XCompoundTextStyle,
                                     &property);

        IMCommitStruct commit_data = {0};
        commit_data.major_code = XIM_COMMIT;
        commit_data.connect_id = connection->xim_connect_id;
        commit_data.icid       = connection->id;
        commit_data.flag       = XimLookupChars;
        commit_data.commit_string = (gchar *) property.value;
        IMCommitString (xims, (XPointer) &commit_data);

        XFree (property.value);
      }
      break;
    default:
      g_warning ("Unknown type: %d", connection->type);
      break;
  }
}

gboolean
aim_connection_emit_retrieve_surrounding (AimConnection *connection)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  aim_send_message (connection->socket, AIM_MESSAGE_RETRIEVE_SURROUNDING,
                    NULL, 0, NULL);
  aim_result_iteration_until (connection->result, NULL,
                              AIM_MESSAGE_RETRIEVE_SURROUNDING_REPLY);

  if (connection->result->reply == NULL)
    return FALSE;

  return *(gboolean *) (connection->result->reply->data);
}

gboolean
aim_connection_emit_delete_surrounding (AimConnection *connection,
                                        gint           offset,
                                        gint           n_chars)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  gint *data = g_malloc (2 * sizeof (gint));
  data[0] = offset;
  data[1] = n_chars;

  aim_send_message (connection->socket, AIM_MESSAGE_DELETE_SURROUNDING,
                    data, 2 * sizeof (gint), g_free);
  aim_result_iteration_until (connection->result, NULL,
                              AIM_MESSAGE_DELETE_SURROUNDING_REPLY);

  if (connection->result->reply == NULL)
    return FALSE;

  return *(gboolean *) (connection->result->reply->data);
}

void
aim_connection_xim_set_cursor_location (AimConnection *connection,
                                        Display       *display)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  AimRectangle preedit_area = connection->cursor_area;

  Window target;

  if (connection->focus_window)
    target = connection->focus_window;
  else
    target = connection->client_window;

  if (target)
  {
    XWindowAttributes xwa;
    Window child;

    XGetWindowAttributes (display, target, &xwa);
    XTranslateCoordinates (display, target,
                           xwa.root,
                           preedit_area.x,
                           preedit_area.y,
                           &preedit_area.x,
                           &preedit_area.y,
                           &child);
  }

  aim_connection_set_cursor_location (connection, &preedit_area);
}
