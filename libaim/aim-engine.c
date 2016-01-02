/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
 * aim-engine.c
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

#include "aim-engine.h"
#include "aim-private.h"

enum
{
  PROP_0,
  PROP_SERVER
};

G_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE (AimEngine, aim_engine, G_TYPE_OBJECT);

static void
aim_engine_set_property (GObject      *object,
                         guint         prop_id,
                         const GValue *value,
                         GParamSpec   *pspec)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_if_fail (AIM_IS_ENGINE (object));

  AimEngine *engine = AIM_ENGINE (object);

  switch (prop_id)
  {
    case PROP_SERVER:
      engine->priv->server = g_value_get_object (value);
      g_object_notify_by_pspec (object, pspec);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
aim_engine_get_property (GObject    *object,
                         guint       prop_id,
                         GValue     *value,
                         GParamSpec *pspec)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_if_fail (AIM_IS_ENGINE (object));

  AimEngine *engine = AIM_ENGINE (object);

  switch (prop_id)
  {
    case PROP_SERVER:
      g_value_set_object (value, engine->priv->server);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

void aim_engine_reset (AimEngine     *engine,
                       AimConnection *target)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_if_fail (AIM_IS_ENGINE (engine));

  AimEngineClass *class = AIM_ENGINE_GET_CLASS (engine);

  if (class->reset)
    class->reset (engine, target);
}

void aim_engine_focus_in (AimEngine *engine)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_if_fail (AIM_IS_ENGINE (engine));

  AimEngineClass *class = AIM_ENGINE_GET_CLASS (engine);

  if (class->focus_in)
    class->focus_in (engine);
}

void aim_engine_focus_out (AimEngine     *engine,
                           AimConnection *target)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_if_fail (AIM_IS_ENGINE (engine));

  AimEngineClass *class = AIM_ENGINE_GET_CLASS (engine);

  if (class->focus_out)
    class->focus_out (engine, target);
}

gboolean aim_engine_filter_event (AimEngine     *engine,
                                  AimConnection *target,
                                  AimEvent      *event)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  AimEngineClass *class = AIM_ENGINE_GET_CLASS (engine);

  return class->filter_event (engine, target, event);
}

gboolean aim_engine_real_filter_event (AimEngine     *engine,
                                       AimConnection *target,
                                       AimEvent      *event)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return FALSE;
}

void
aim_engine_get_preedit_string (AimEngine  *engine,
                               gchar     **str,
                               gint       *cursor_pos)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_if_fail (AIM_IS_ENGINE (engine));

  AimEngineClass *class = AIM_ENGINE_GET_CLASS (engine);
  class->get_preedit_string (engine, str, cursor_pos);
}

void
aim_engine_set_surrounding (AimEngine  *engine,
                            const char *text,
                            gint        len,
                            gint        cursor_index)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_if_fail (AIM_IS_ENGINE (engine));
  g_return_if_fail (text != NULL || len == 0);

  AimEngineClass *class = AIM_ENGINE_GET_CLASS (engine);

  if (class->set_surrounding)
    class->set_surrounding (engine, text, len, cursor_index);
}

gboolean
aim_engine_get_surrounding (AimEngine      *engine,
                            AimConnection  *target,
                            gchar         **text,
                            gint           *cursor_index)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  gboolean retval = FALSE;
  AimEngineClass *class = AIM_ENGINE_GET_CLASS (engine);

  if (class->get_surrounding)
    retval = class->get_surrounding (engine, target, text, cursor_index);

  return retval;
}

void
aim_engine_set_cursor_location (AimEngine          *engine,
                                const AimRectangle *area)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_if_fail (AIM_IS_ENGINE (engine));

  AimEngineClass *class = AIM_ENGINE_GET_CLASS (engine);

  if (class->set_cursor_location)
    class->set_cursor_location (engine, area);
}

void
aim_engine_set_english_mode (AimEngine *engine,
                             gboolean   is_english_mode)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  AimEngineClass *class = AIM_ENGINE_GET_CLASS (engine);

  if (class->set_english_mode)
    class->set_english_mode (engine, is_english_mode);
}

gboolean
aim_engine_get_english_mode (AimEngine *engine)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  AimEngineClass *class = AIM_ENGINE_GET_CLASS (engine);

  if (class->get_english_mode)
    return class->get_english_mode (engine);

  return TRUE;
}

void
aim_engine_emit_preedit_start (AimEngine     *engine,
                               AimConnection *target)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  aim_connection_emit_preedit_start (target);
}

void
aim_engine_emit_preedit_changed (AimEngine     *engine,
                                 AimConnection *target,
                                 const gchar   *preedit_string,
                                 gint           cursor_pos)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  aim_connection_emit_preedit_changed (target, preedit_string, cursor_pos);
}

void
aim_engine_emit_preedit_end (AimEngine     *engine,
                             AimConnection *target)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  aim_connection_emit_preedit_end (target);
}

void
aim_engine_emit_commit (AimEngine     *engine,
                        AimConnection *target,
                        const gchar   *text)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  aim_connection_emit_commit (target, text);
}

gboolean
aim_engine_emit_delete_surrounding (AimEngine     *engine,
                                    AimConnection *target,
                                    gint           offset,
                                    gint           n_chars)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return aim_connection_emit_delete_surrounding (target, offset, n_chars);
}

gboolean
aim_engine_emit_retrieve_surrounding (AimEngine     *engine,
                                      AimConnection *target)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return aim_connection_emit_retrieve_surrounding (target);
}

void
aim_engine_emit_engine_changed (AimEngine     *engine,
                                AimConnection *target)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_signal_emit_by_name (target, "engine-changed",
                         aim_engine_get_name (engine));
}

static void
aim_engine_init (AimEngine *engine)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  engine->priv = aim_engine_get_instance_private (engine);
}

static void
aim_engine_finalize (GObject *object)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  AimEngine *engine = AIM_ENGINE (object);

  g_free (engine->priv->surrounding_text);

  G_OBJECT_CLASS (aim_engine_parent_class)->finalize (object);
}


static void
aim_engine_real_get_preedit_string (AimEngine  *engine,
                                    gchar     **str,
                                    gint       *cursor_pos)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (str)
    *str = g_strdup ("");

  if (cursor_pos)
    *cursor_pos = 0;
}

static void
aim_engine_real_set_surrounding (AimEngine  *engine,
                                 const char *text,
                                 gint        len,
                                 gint        cursor_index)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_free (engine->priv->surrounding_text);
  engine->priv->surrounding_text         = g_strndup (text, len);
  engine->priv->surrounding_cursor_index = cursor_index;
}

static gboolean
aim_engine_real_get_surrounding (AimEngine      *engine,
                                 AimConnection  *target,
                                 gchar         **text,
                                 gint           *cursor_index)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  gboolean retval = aim_engine_emit_retrieve_surrounding (engine, target);

  if (retval)
  {
    if (engine->priv->surrounding_text)
      *text = g_strdup (engine->priv->surrounding_text);
    else
      *text = g_strdup ("");

    *cursor_index = engine->priv->surrounding_cursor_index;
  }
  else
  {
    *text = NULL;
    *cursor_index = 0;
  }

  return retval;
}

void
aim_engine_update_candidate_window (AimEngine    *engine,
                                    const gchar **strv)
{
  aim_candidate_update_window (engine->priv->server->candidate, strv);
}

void
aim_engine_show_candidate_window (AimEngine     *engine,
                                  AimConnection *target)
{
  aim_candidate_show_window (engine->priv->server->candidate, target);
}

void
aim_engine_hide_candidate_window (AimEngine *engine)
{
  aim_candidate_hide_window (engine->priv->server->candidate);
}

gboolean aim_engine_is_candidate_window_visible (AimEngine *engine)
{
  return aim_candidate_is_window_visible (engine->priv->server->candidate);
}

void
aim_engine_select_previous_candidate_item (AimEngine *engine)
{
  aim_candidate_select_previous_item (engine->priv->server->candidate);
}

void
aim_engine_select_next_candidate_item (AimEngine *engine)
{
  aim_candidate_select_next_item (engine->priv->server->candidate);
}

void
aim_engine_select_page_up_candidate_item (AimEngine *engine)
{
  aim_candidate_select_page_up_item (engine->priv->server->candidate);
}

void
aim_engine_select_page_down_candidate_item (AimEngine *engine)
{
  aim_candidate_select_page_down_item (engine->priv->server->candidate);
}

gchar *
aim_engine_get_selected_candidate_text (AimEngine *engine)
{
  return aim_candidate_get_selected_text (engine->priv->server->candidate);
}

gint
aim_engine_get_selected_candidate_index (AimEngine *engine)
{
  return aim_candidate_get_selected_index (engine->priv->server->candidate);
}

const gchar *
aim_engine_get_id (AimEngine *engine)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return AIM_ENGINE_GET_CLASS (engine)->get_id (engine);
}

static const gchar *
aim_engine_real_get_id (AimEngine *engine)
{
  g_critical (G_STRLOC ": %s: You should implement your_engine_get_id ()",
              G_STRFUNC);
  return NULL;
}

const gchar *
aim_engine_get_name (AimEngine *engine)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return AIM_ENGINE_GET_CLASS (engine)->get_name (engine);
}

static const gchar *
aim_engine_real_get_name (AimEngine *engine)
{
  /* FIXME */
  g_error ("You should implement your_engine_get_name ()");

  return NULL;
}

static void
aim_engine_class_init (AimEngineClass *class)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GObjectClass *object_class = G_OBJECT_CLASS (class);

  object_class->finalize     = aim_engine_finalize;

  object_class->set_property = aim_engine_set_property;
  object_class->get_property = aim_engine_get_property;

  class->filter_event        = aim_engine_real_filter_event;
  class->get_preedit_string  = aim_engine_real_get_preedit_string;
  class->set_surrounding     = aim_engine_real_set_surrounding;
  class->get_surrounding     = aim_engine_real_get_surrounding;
 /* TODO: maybe get_engine_info */
  class->get_id              = aim_engine_real_get_id;
  class->get_name            = aim_engine_real_get_name;

  g_object_class_install_property (object_class,
                                   PROP_SERVER,
                                   g_param_spec_object ("server",
                                                        "server",
                                                        "server",
                                                        AIM_TYPE_SERVER,
                                                        G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
}
