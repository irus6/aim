/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
 * aim-agent.h
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

#ifndef __AIM_AGENT_H__
#define __AIM_AGENT_H__

#if !defined (__AIM_H_INSIDE__) && !defined (AIM_COMPILATION)
#error "Only <aim.h> can be included directly."
#endif

#include <gio/gio.h>
#include "aim-message.h"

G_BEGIN_DECLS

#define AIM_TYPE_AGENT             (aim_agent_get_type ())
#define AIM_AGENT(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), AIM_TYPE_AGENT, AimAgent))
#define AIM_AGENT_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), AIM_TYPE_AGENT, AimAgentClass))
#define AIM_IS_AGENT(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), AIM_TYPE_AGENT))
#define AIM_IS_AGENT_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), AIM_TYPE_AGENT))
#define AIM_AGENT_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), AIM_TYPE_AGENT, AimAgentClass))

typedef struct _AimAgent      AimAgent;
typedef struct _AimAgentClass AimAgentClass;

struct _AimAgent
{
  GObject parent_instance;

  /*< private >*/
  GSocketConnection *connection;
  AimMessage        *reply;
  GSource           *source;
};

struct _AimAgentClass
{
  /*< private >*/
  GObjectClass parent_class;

  /*< public >*/
  /* Signals */
  void (*engine_changed) (AimAgent    *context,
                          const gchar *str);
  void (*disconnected)   (AimAgent    *context);
};

GType     aim_agent_get_type          (void) G_GNUC_CONST;
AimAgent *aim_agent_new               (void);
gboolean  aim_agent_connect_to_server (AimAgent *agent);

G_END_DECLS

#endif /* __AIM_AGENT_H__ */

