/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
 * aim-engine.h
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

#ifndef __AIM_ENGINE_H__
#define __AIM_ENGINE_H__

#if !defined (__AIM_H_INSIDE__) && !defined (AIM_COMPILATION)
#error "Only <aim.h> can be included directly."
#endif

#include <glib-object.h>
#include "aim-events.h"
#include "aim-types.h"
#include "aim-connection.h"

G_BEGIN_DECLS

#define AIM_TYPE_ENGINE             (aim_engine_get_type ())
#define AIM_ENGINE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), AIM_TYPE_ENGINE, AimEngine))
#define AIM_ENGINE_CLASS(class)     (G_TYPE_CHECK_CLASS_CAST ((class), AIM_TYPE_ENGINE, AimEngineClass))
#define AIM_IS_ENGINE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), AIM_TYPE_ENGINE))
#define AIM_IS_ENGINE_CLASS(class)  (G_TYPE_CHECK_CLASS_TYPE ((class), AIM_TYPE_ENGINE))
#define AIM_ENGINE_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), AIM_TYPE_ENGINE, AimEngineClass))

typedef struct _AimEngineInfo AimEngineInfo;

/* TODO */
struct _AimEngineInfo
{
  const gchar *engine_name;
};

typedef struct _AimConnection    AimConnection;

typedef struct _AimEngine        AimEngine;
typedef struct _AimEngineClass   AimEngineClass;
typedef struct _AimEnginePrivate AimEnginePrivate;

struct _AimEngine
{
  GObject parent_instance;
  AimEnginePrivate *priv;
};

struct _AimEngineClass
{
  /*< private >*/
  GObjectClass parent_class;

  /*< public >*/
  /* Virtual functions */
  gboolean (* filter_event)       (AimEngine      *engine,
                                   AimConnection  *connection,
                                   AimEvent       *event);
  void     (* get_preedit_string) (AimEngine      *engine,
                                   gchar         **str,
                                   gint           *cursor_pos);
  void     (* reset)              (AimEngine      *engine,
                                   AimConnection  *target);
  void     (* focus_in)           (AimEngine      *engine);
  void     (* focus_out)          (AimEngine      *engine,
                                   AimConnection  *target);
  void     (* set_surrounding)    (AimEngine      *engine,
                                   const char     *text,
                                   gint            len,
                                   gint            cursor_index);
  gboolean (* get_surrounding)    (AimEngine      *engine,
                                   AimConnection  *target,
                                   gchar         **text,
                                   gint           *cursor_index);
  void     (* set_cursor_location)(AimEngine      *engine,
                                   const AimRectangle *area);
  void     (* set_english_mode)   (AimEngine      *engine,
                                   gboolean        is_english_mode);
  gboolean (* get_english_mode)   (AimEngine      *engine);

  void     (* candidate_clicked)  (AimEngine      *engine,
                                   AimConnection  *target,
                                   gchar          *text,
                                   gint            index);
  /* info */
  const gchar * (* get_id)        (AimEngine      *engine);
  const gchar * (* get_name)      (AimEngine      *engine);
};

GType    aim_engine_get_type                  (void) G_GNUC_CONST;
gboolean aim_engine_filter_event              (AimEngine      *engine,
                                               AimConnection  *target,
                                               AimEvent       *event);
void     aim_engine_reset                     (AimEngine      *engine,
                                               AimConnection  *target);
void     aim_engine_focus_in                  (AimEngine      *engine);
void     aim_engine_focus_out                 (AimEngine      *engine,
                                               AimConnection  *target);
void     aim_engine_get_preedit_string        (AimEngine      *engine,
                                               gchar         **str,
                                               gint           *cursor_pos);
void     aim_engine_set_surrounding           (AimEngine      *engine,
                                               const char     *text,
                                               gint            len,
                                               gint            cursor_index);
gboolean aim_engine_get_surrounding           (AimEngine      *engine,
                                               AimConnection  *target,
                                               gchar         **text,
                                               gint           *cursor_index);
void     aim_engine_set_cursor_location       (AimEngine      *engine,
                                               const AimRectangle *area);
void     aim_engine_set_english_mode          (AimEngine      *engine,
                                               gboolean        is_english_mode);
gboolean aim_engine_get_english_mode          (AimEngine      *engine);
/* signals */
void     aim_engine_emit_preedit_start        (AimEngine      *engine,
                                               AimConnection  *target);
void     aim_engine_emit_preedit_changed      (AimEngine      *engine,
                                               AimConnection  *target,
                                               const gchar    *preedit_string,
                                               gint            cursor_pos);
void     aim_engine_emit_preedit_end          (AimEngine      *engine,
                                               AimConnection  *target);
void     aim_engine_emit_commit               (AimEngine      *engine,
                                               AimConnection  *target,
                                               gchar const    *text);
gboolean aim_engine_emit_retrieve_surrounding (AimEngine      *engine,
                                               AimConnection  *target);
gboolean aim_engine_emit_delete_surrounding   (AimEngine      *engine,
                                               AimConnection  *target,
                                               gint            offset,
                                               gint            n_chars);
void     aim_engine_emit_engine_changed       (AimEngine      *engine,
                                               AimConnection  *target);
/* candidate */
void     aim_engine_update_candidate_window         (AimEngine  *engine,
                                                     const gchar **strv);
void     aim_engine_show_candidate_window           (AimEngine  *engine,
                                                     AimConnection *target);
void     aim_engine_hide_candidate_window           (AimEngine  *engine);
void     aim_engine_select_previous_candidate_item  (AimEngine  *engine);
void     aim_engine_select_next_candidate_item      (AimEngine  *engine);
void     aim_engine_select_page_up_candidate_item   (AimEngine  *engine);
void     aim_engine_select_page_down_candidate_item (AimEngine  *engine);
gchar   *aim_engine_get_selected_candidate_text     (AimEngine  *engine);
/* info */
const gchar *aim_engine_get_id   (AimEngine *engine);
const gchar *aim_engine_get_name (AimEngine *engine);

G_END_DECLS

#endif /* __AIM_ENGINE_H__ */
