/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
 * aim-message.h
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

#ifndef __AIM_MESSAGE_H__
#define __AIM_MESSAGE_H__

#if !defined (__AIM_H_INSIDE__) && !defined (AIM_COMPILATION)
#error "Only <aim.h> can be included directly."
#endif

#include <glib-object.h>
#include "aim-events.h"

G_BEGIN_DECLS

typedef struct _AimMessage       AimMessage;
typedef struct _AimMessageHeader AimMessageHeader;

typedef enum
{
  AIM_MESSAGE_NONE = 0,
  /* im methods */
  AIM_MESSAGE_CONNECT,
  AIM_MESSAGE_CONNECT_REPLY,
  AIM_MESSAGE_FILTER_EVENT,
  AIM_MESSAGE_FILTER_EVENT_REPLY,
  AIM_MESSAGE_RESET,
  AIM_MESSAGE_RESET_REPLY,
  AIM_MESSAGE_FOCUS_IN,
  AIM_MESSAGE_FOCUS_IN_REPLY,
  AIM_MESSAGE_FOCUS_OUT,
  AIM_MESSAGE_FOCUS_OUT_REPLY,
  AIM_MESSAGE_SET_SURROUNDING,
  AIM_MESSAGE_SET_SURROUNDING_REPLY,
  AIM_MESSAGE_GET_SURROUNDING,
  AIM_MESSAGE_GET_SURROUNDING_REPLY,
  AIM_MESSAGE_SET_CURSOR_LOCATION,
  AIM_MESSAGE_SET_CURSOR_LOCATION_REPLY,
  AIM_MESSAGE_SET_USE_PREEDIT,
  AIM_MESSAGE_SET_USE_PREEDIT_REPLY,
  /* context signals */
  AIM_MESSAGE_PREEDIT_START,
  AIM_MESSAGE_PREEDIT_START_REPLY,
  AIM_MESSAGE_PREEDIT_END,
  AIM_MESSAGE_PREEDIT_END_REPLY,
  AIM_MESSAGE_PREEDIT_CHANGED,
  AIM_MESSAGE_PREEDIT_CHANGED_REPLY,
  AIM_MESSAGE_COMMIT,
  AIM_MESSAGE_COMMIT_REPLY,
  AIM_MESSAGE_RETRIEVE_SURROUNDING,
  AIM_MESSAGE_RETRIEVE_SURROUNDING_REPLY,
  AIM_MESSAGE_DELETE_SURROUNDING,
  AIM_MESSAGE_DELETE_SURROUNDING_REPLY,
  AIM_MESSAGE_ENGINE_CHANGED,

  AIM_MESSAGE_ERROR
} AimMessageType;

struct _AimMessageHeader
{
  AimMessageType type;
  guint16        data_len;
};

struct _AimMessage
{
  AimMessageHeader *header;
  gchar            *data;
  GDestroyNotify    data_destroy_func;
  gint              ref_count;
};

AimMessage   *aim_message_new              (void);
AimMessage   *aim_message_new_full         (AimMessageType  type,
                                            gpointer        data,
                                            guint16         data_len,
                                            GDestroyNotify  data_destroy_func);
AimMessage   *aim_message_ref              (AimMessage     *message);
void          aim_message_unref            (AimMessage     *message);
const AimMessageHeader *
              aim_message_get_header       (AimMessage     *message);
guint16       aim_message_get_header_size  (void);
void          aim_message_set_body         (AimMessage     *message,
                                            gchar          *data,
                                            guint16         data_len,
                                            GDestroyNotify  data_destroy_func);
const gchar  *aim_message_get_body         (AimMessage     *message);
guint16       aim_message_get_body_size    (AimMessage     *message);
const gchar  *aim_message_get_name         (AimMessage     *message);
const gchar  *aim_message_get_name_by_type (AimMessageType  type);

G_END_DECLS

#endif /* __AIM_MESSAGE_H__ */
