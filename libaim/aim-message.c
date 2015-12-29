/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
 * aim-message.c
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

#include "aim-message.h"
#include "aim-types.h"
#include "aim-enum-types.h"
#include <string.h>

AimMessage *
aim_message_new ()
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return aim_message_new_full (AIM_MESSAGE_NONE, NULL, 0, NULL);
}

AimMessage *
aim_message_new_full (AimMessageType type,
                      gpointer       data,
                      guint16        data_len,
                      GDestroyNotify data_destroy_func)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  AimMessage *message;

  message                    = g_slice_new0 (AimMessage);
  message->header            = g_slice_new0 (AimMessageHeader);
  message->header->type      = type;
  message->header->data_len  = data_len;
  message->data              = data;
  message->data_destroy_func = data_destroy_func;
  message->ref_count = 1;

  return message;
}

AimMessage *
aim_message_ref (AimMessage *message)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_val_if_fail (message != NULL, NULL);

  g_atomic_int_inc (&message->ref_count);

  return message;
}

void
aim_message_unref (AimMessage *message)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  if (G_UNLIKELY (message == NULL))
    return;

  if (g_atomic_int_dec_and_test (&message->ref_count))
  {
    g_slice_free (AimMessageHeader, message->header);

    if (message->data_destroy_func)
      message->data_destroy_func (message->data);

    g_slice_free (AimMessage, message);
  }
}

const AimMessageHeader *
aim_message_get_header (AimMessage *message)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return message->header;
}

guint16
aim_message_get_header_size ()
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return sizeof (AimMessageHeader);
}

void
aim_message_set_body (AimMessage     *message,
                      gchar          *data,
                      guint16         data_len,
                      GDestroyNotify  data_destroy_func)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  message->data              = data;
  message->header->data_len  = data_len;
  message->data_destroy_func = data_destroy_func;
}

const gchar *
aim_message_get_body (AimMessage *message)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return message->data;
}

guint16
aim_message_get_body_size (AimMessage *message)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return message->header->data_len;
}

const gchar *aim_message_get_name (AimMessage *message)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GEnumClass *enum_class = (GEnumClass *) g_type_class_ref (AIM_TYPE_MESSAGE_TYPE);
  GEnumValue *enum_value = g_enum_get_value (enum_class, message->header->type);
  g_type_class_unref (enum_class);

  return enum_value ? enum_value->value_name : NULL;
}

const gchar *aim_message_get_name_by_type (AimMessageType type)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GEnumClass *enum_class = (GEnumClass *) g_type_class_ref (AIM_TYPE_MESSAGE_TYPE);
  GEnumValue *enum_value = g_enum_get_value (enum_class, type);
  g_type_class_unref (enum_class);

  return enum_value ? enum_value->value_name : NULL;
}
