/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
 * aim-private.h
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

#ifndef __AIM_PRIVATE_H__
#define __AIM_PRIVATE_H__

#if !defined (__AIM_H_INSIDE__) && !defined (AIM_COMPILATION)
#error "Only <aim.h> can be included directly."
#endif

#include <glib-object.h>
#include "aim-server.h"
#include "aim-message.h"

G_BEGIN_DECLS

typedef struct _AimServer AimServer;

typedef struct _AimEnginePrivate AimEnginePrivate;

struct _AimEnginePrivate
{
  AimServer  *server;
  gchar      *surrounding_text;
  gint        surrounding_cursor_index;
};

typedef struct _AimResult AimResult;

struct _AimResult
{
  gboolean    is_dispatched;
  AimMessage *reply;
};

void        aim_send_message           (GSocket        *socket,
                                        AimMessageType  type,
                                        gpointer        data,
                                        guint16         data_len,
                                        GDestroyNotify  data_destroy_func);
AimMessage *aim_recv_message           (GSocket        *socket);
void        aim_log_default_handler    (const gchar    *log_domain,
                                        GLogLevelFlags  log_level,
                                        const gchar    *message,
                                        gboolean       *debug);
void        aim_result_iteration_until (AimResult      *result,
                                        GMainContext   *main_context,
                                        AimMessageType  type);
G_END_DECLS

#endif /* __AIM_PRIVATE_H__ */
