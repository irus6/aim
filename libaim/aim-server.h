/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
 * aim-server.h
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

#ifndef __AIM_SERVER_H__
#define __AIM_SERVER_H__

#if !defined (__AIM_H_INSIDE__) && !defined (AIM_COMPILATION)
#error "Only <aim.h> can be included directly."
#endif

#include <glib-object.h>
#include "aim-module-manager.h"
#include <gio/gio.h>
#include "aim-types.h"
#include "aim-candidate.h"
#include "aim-engine.h"

G_BEGIN_DECLS

#define AIM_TYPE_SERVER             (aim_server_get_type ())
#define AIM_SERVER(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), AIM_TYPE_SERVER, AimServer))
#define AIM_SERVER_CLASS(class)     (G_TYPE_CHECK_CLASS_CAST ((class), AIM_TYPE_SERVER, AimServerClass))
#define AIM_IS_SERVER(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), AIM_TYPE_SERVER))
#define AIM_IS_SERVER_CLASS(class)  (G_TYPE_CHECK_CLASS_TYPE ((class), AIM_TYPE_SERVER))
#define AIM_SERVER_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), AIM_TYPE_SERVER, AimServerClass))

typedef struct _AimEngine      AimEngine;
typedef struct _AimCandidate   AimCandidate;

typedef struct _AimServer      AimServer;
typedef struct _AimServerClass AimServerClass;

struct _AimServer
{
  GObject parent_instance;

  GMainContext      *main_context;
  AimModuleManager  *module_manager;
  GList             *instances;
  GSocketListener   *listener;
  GHashTable        *connections;
  GList             *agents_list;
  AimKey           **hotkeys;
  AimCandidate      *candidate;
  GSource           *xevent_source;
  guint16            next_id;

  gchar     *address;
  gboolean   active;
  gboolean   is_using_listener;
  gulong     run_signal_handler_id;
};

struct _AimServerClass
{
  GObjectClass parent_class;
};

GType      aim_server_get_type           (void) G_GNUC_CONST;

AimServer *aim_server_new                (const gchar  *address,
                                          GError      **error);
void       aim_server_start              (AimServer    *server);
void       aim_server_stop               (AimServer    *server);
AimEngine *aim_server_get_default_engine (AimServer    *server);
AimEngine *aim_server_get_next_instance  (AimServer    *server,
                                          AimEngine    *engine);
AimEngine *aim_server_get_instance       (AimServer    *server,
                                          const gchar  *module_name);
G_END_DECLS

#endif /* __AIM_SERVER_H__ */

