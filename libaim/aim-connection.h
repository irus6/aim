/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
 * aim-connection.h
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

#ifndef __AIM_CONNECTION_H__
#define __AIM_CONNECTION_H__

#if !defined (__AIM_H_INSIDE__) && !defined (AIM_COMPILATION)
#error "Only <aim.h> can be included directly."
#endif

#include <glib-object.h>
#include "aim-engine.h"
#include "aim-server.h"
#include "aim-private.h"
#include <X11/Xlib.h>

G_BEGIN_DECLS

#define AIM_TYPE_CONNECTION             (aim_connection_get_type ())
#define AIM_CONNECTION(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), AIM_TYPE_CONNECTION, AimConnection))
#define AIM_CONNECTION_CLASS(class)     (G_TYPE_CHECK_CLASS_CAST ((class), AIM_TYPE_CONNECTION, AimConnectionClass))
#define AIM_IS_CONNECTION(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), AIM_TYPE_CONNECTION))
#define AIM_IS_CONNECTION_CLASS(class)  (G_TYPE_CHECK_CLASS_TYPE ((class), AIM_TYPE_CONNECTION))
#define AIM_CONNECTION_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), AIM_TYPE_CONNECTION, AimConnectionClass))

typedef struct _AimServer AimServer;
typedef struct _AimEngine AimEngine;
typedef struct _AimResult AimResult;

typedef struct _AimConnection      AimConnection;
typedef struct _AimConnectionClass AimConnectionClass;

struct _AimConnection
{
  GObject parent_instance;

  AimConnectionType  type;
  AimEngine         *engine;
  guint16            id;
  gboolean           use_preedit;
  gboolean           is_english_mode;
  AimRectangle       cursor_area;
  AimServer         *server;
  GSocket           *socket;
  AimResult         *result;
  GSource           *source;
  GSocketConnection *socket_connection;
  /* XIM */
  guint16            xim_connect_id;
  gint               xim_preedit_length;
  AimPreeditState    preedit_state;
  gpointer           cb_user_data;
  Window             client_window;
  Window             focus_window;
};

struct _AimConnectionClass
{
  GObjectClass parent_class;
  /*< public >*/
  /* Signals */
  void     (*preedit_start)        (AimConnection *connection);
  void     (*preedit_end)          (AimConnection *connection);
  void     (*preedit_changed)      (AimConnection *connection);
  void     (*commit)               (AimConnection *connection,
                                    const gchar   *str);
  gboolean (*retrieve_surrounding) (AimConnection *connection);
  gboolean (*delete_surrounding)   (AimConnection *connection,
                                    gint           offset,
                                    gint           n_chars);
  void     (*engine_changed)       (AimConnection *connection,
                                    const gchar   *str);
};

GType          aim_connection_get_type                  (void) G_GNUC_CONST;
AimConnection *aim_connection_new                       (AimConnectionType type,
                                                         AimEngine        *engine,
                                                         gpointer          cb_user_data);
void           aim_connection_set_engine                (AimConnection    *connection,
                                                         AimEngine        *engine);
guint16        aim_connection_get_id                    (AimConnection    *connection);
gboolean       aim_connection_filter_event              (AimConnection    *connection,
                                                         AimEvent         *event);
void           aim_connection_get_preedit_string        (AimConnection    *connection,
                                                         gchar           **str,
                                                         gint             *cursor_pos);
void           aim_connection_reset                     (AimConnection    *connection);
void           aim_connection_focus_in                  (AimConnection    *connection);
void           aim_connection_focus_out                 (AimConnection    *connection);
void           aim_connection_set_surrounding           (AimConnection    *connection,
                                                         const char       *text,
                                                         gint              len,
                                                         gint              cursor_index);
gboolean       aim_connection_get_surrounding           (AimConnection    *connection,
                                                         gchar           **text,
                                                         gint             *cursor_index);
void           aim_connection_xim_set_cursor_location   (AimConnection    *connection,
                                                         Display          *display);
void           aim_connection_set_cursor_location       (AimConnection    *connection,
                                                         const AimRectangle *area);
void           aim_connection_set_use_preedit           (AimConnection    *connection,
                                                         gboolean          use_preedit);
/* signals */
void           aim_connection_emit_preedit_start        (AimConnection    *connection);
void           aim_connection_emit_preedit_changed      (AimConnection    *connection,
                                                         const gchar      *preedit_string,
                                                         gint              cursor_pos);
void           aim_connection_emit_preedit_end          (AimConnection    *connection);
void           aim_connection_emit_commit               (AimConnection    *connection,
                                                         const gchar      *text);
gboolean       aim_connection_emit_retrieve_surrounding (AimConnection    *connection);
gboolean       aim_connection_emit_delete_surrounding   (AimConnection    *connection,
                                                         gint              offset,
                                                         gint              n_chars);
G_END_DECLS

#endif /* __AIM_CONNECTION_H__ */
