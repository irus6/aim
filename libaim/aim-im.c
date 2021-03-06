/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
 * aim-im.c
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

#include "aim-im.h"
#include "aim-events.h"
#include "aim-types.h"
#include "aim-module-manager.h"
#include "aim-key-syms.h"
#include "aim-marshalers.h"
#include <gio/gunixsocketaddress.h>
#include "aim-message.h"
#include "aim-private.h"
#include <string.h>

enum {
  PREEDIT_START,
  PREEDIT_END,
  PREEDIT_CHANGED,
  COMMIT,
  RETRIEVE_SURROUNDING,
  DELETE_SURROUNDING,
  LAST_SIGNAL
};

static guint im_signals[LAST_SIGNAL] = { 0 };
static GMainContext *aim_im_sockets_context = NULL;
static guint         aim_im_sockets_context_ref_count = 0;

G_DEFINE_TYPE (AimIM, aim_im, G_TYPE_OBJECT);

static gboolean
on_incoming_message (GSocket      *socket,
                     GIOCondition  condition,
                     gpointer      user_data)
{
  g_debug (G_STRLOC ": %s: socket fd:%d", G_STRFUNC, g_socket_get_fd (socket));

  AimIM *im = AIM_IM (user_data);
  aim_message_unref (im->result->reply);
  im->result->is_dispatched = TRUE;

  if (condition & (G_IO_HUP | G_IO_ERR))
  {
    /* Because two GSource is created over one socket,
     * when G_IO_HUP | G_IO_ERR, callback can run two times.
     * the following code avoid that callback runs two times. */
    GSource *source = g_main_current_source ();

    if (source == im->default_context_source)
      g_source_destroy (im->sockets_context_source);
    else if (source == im->sockets_context_source)
      g_source_destroy (im->default_context_source);

    if (!g_socket_is_closed (socket))
      g_socket_close (socket, NULL);

    im->result->reply    = NULL;

    g_critical (G_STRLOC ": %s: G_IO_HUP | G_IO_ERR", G_STRFUNC);

    return G_SOURCE_REMOVE;
  }

  AimMessage *message;
  message = aim_recv_message (socket);
  im->result->reply = message;
  gboolean retval;

  if (G_UNLIKELY (message == NULL))
  {
    g_critical (G_STRLOC ": NULL message");
    return G_SOURCE_CONTINUE;
  }

  switch (message->header->type)
  {
    /* reply */
    case AIM_MESSAGE_FILTER_EVENT_REPLY:
    case AIM_MESSAGE_RESET_REPLY:
    case AIM_MESSAGE_FOCUS_IN_REPLY:
    case AIM_MESSAGE_FOCUS_OUT_REPLY:
    case AIM_MESSAGE_SET_SURROUNDING_REPLY:
    case AIM_MESSAGE_GET_SURROUNDING_REPLY:
    case AIM_MESSAGE_SET_CURSOR_LOCATION_REPLY:
    case AIM_MESSAGE_SET_USE_PREEDIT_REPLY:
      break;
    /* signals */
    case AIM_MESSAGE_PREEDIT_START:
      g_signal_emit_by_name (im, "preedit-start");
      aim_send_message (socket, AIM_MESSAGE_PREEDIT_START_REPLY, NULL, 0, NULL);
      break;
    case AIM_MESSAGE_PREEDIT_END:
      g_signal_emit_by_name (im, "preedit-end");
      aim_send_message (socket, AIM_MESSAGE_PREEDIT_END_REPLY, NULL, 0, NULL);
      break;
    case AIM_MESSAGE_PREEDIT_CHANGED:
      g_free (im->preedit_string);
      im->preedit_string = g_strndup (message->data,
                                      message->header->data_len - 1 - sizeof (gint));
      im->cursor_pos = *(gint *) (message->data +
                                  message->header->data_len - sizeof (gint));
      g_signal_emit_by_name (im, "preedit-changed");
      aim_send_message (socket, AIM_MESSAGE_PREEDIT_CHANGED_REPLY,
                        NULL, 0, NULL);
      break;
    case AIM_MESSAGE_COMMIT:
      aim_message_ref (message);
      g_signal_emit_by_name (im, "commit", (const gchar *) message->data);
      aim_message_unref (message);
      aim_send_message (socket, AIM_MESSAGE_COMMIT_REPLY, NULL, 0, NULL);
      break;
    case AIM_MESSAGE_RETRIEVE_SURROUNDING:
      g_signal_emit_by_name (im, "retrieve-surrounding", &retval);
      aim_send_message (socket, AIM_MESSAGE_RETRIEVE_SURROUNDING_REPLY,
                        &retval, sizeof (gboolean), NULL);
      break;
    case AIM_MESSAGE_DELETE_SURROUNDING:
      aim_message_ref (message);
      g_signal_emit_by_name (im, "delete-surrounding",
                             ((gint *) message->data)[0],
                             ((gint *) message->data)[1], &retval);
      aim_message_unref (message);
      aim_send_message (socket, AIM_MESSAGE_DELETE_SURROUNDING_REPLY,
                        &retval, sizeof (gboolean), NULL);
      break;
    default:
      g_warning (G_STRLOC ": %s: Unknown message type: %d", G_STRFUNC, message->header->type);
      break;
  }

  return G_SOURCE_CONTINUE;
}

void aim_im_focus_out (AimIM *im)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_if_fail (AIM_IS_IM (im));

  GSocket *socket = g_socket_connection_get_socket (im->connection);
  if (!socket || g_socket_is_closed (socket))
  {
    g_warning ("socket is closed");
    return;
  }

  aim_send_message (socket, AIM_MESSAGE_FOCUS_OUT, NULL, 0, NULL);
  aim_result_iteration_until (im->result, aim_im_sockets_context,
                              AIM_MESSAGE_FOCUS_OUT_REPLY);
}

void aim_im_set_cursor_location (AimIM              *im,
                                 const AimRectangle *area)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_if_fail (AIM_IS_IM (im));

  GSocket *socket = g_socket_connection_get_socket (im->connection);
  if (!socket || g_socket_is_closed (socket))
  {
    g_warning ("socket is closed");
    return;
  }

  aim_send_message (socket, AIM_MESSAGE_SET_CURSOR_LOCATION,
                    (gchar *) area, sizeof (AimRectangle), NULL);
  aim_result_iteration_until (im->result, aim_im_sockets_context,
                              AIM_MESSAGE_SET_CURSOR_LOCATION_REPLY);
}

void aim_im_set_use_preedit (AimIM    *im,
                             gboolean  use_preedit)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_if_fail (AIM_IS_IM (im));

  GSocket *socket = g_socket_connection_get_socket (im->connection);
  if (!socket || g_socket_is_closed (socket))
  {
    g_warning ("socket is closed");
    return;
  }

  aim_send_message (socket, AIM_MESSAGE_SET_USE_PREEDIT,
                    (gchar *) &use_preedit, sizeof (gboolean), NULL);
  aim_result_iteration_until (im->result, aim_im_sockets_context,
                              AIM_MESSAGE_SET_USE_PREEDIT_REPLY);
}

gboolean aim_im_get_surrounding (AimIM  *im,
                                 gchar **text,
                                 gint   *cursor_index)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_val_if_fail (AIM_IS_IM (im), FALSE);

  GSocket *socket = g_socket_connection_get_socket (im->connection);
  if (!socket || g_socket_is_closed (socket))
  {
    if (text)
      *text = g_strdup ("");

    if (cursor_index)
      *cursor_index = 0;

    g_warning ("socket is closed");

    return FALSE;
  }

  aim_send_message (socket, AIM_MESSAGE_GET_SURROUNDING, NULL, 0, NULL);
  aim_result_iteration_until (im->result, aim_im_sockets_context,
                              AIM_MESSAGE_GET_SURROUNDING_REPLY);

  if (im->result->reply == NULL)
  {
    if (text)
      *text = g_strdup ("");

    if (cursor_index)
      *cursor_index = 0;

    return FALSE;
  }

  if (text)
    *text = g_strndup (im->result->reply->data,
                       im->result->reply->header->data_len - 1 -
                       sizeof (gint) - sizeof (gboolean));

  if (cursor_index)
  {
    *cursor_index = *(gint *) (im->result->reply->data +
                               im->result->reply->header->data_len -
                               sizeof (gint) - sizeof (gboolean));
  }

  return *(gboolean *) (im->result->reply->data - sizeof (gboolean));
}

void aim_im_set_surrounding (AimIM      *im,
                             const char *text,
                             gint        len,
                             gint        cursor_index)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_if_fail (AIM_IS_IM (im));

  GSocket *socket = g_socket_connection_get_socket (im->connection);
  if (!socket || g_socket_is_closed (socket))
  {
    g_warning ("socket is closed");
    return;
  }

  gchar *data = NULL;
  gint   str_len;

  if (len == -1)
    str_len = strlen (text);
  else
    str_len = len;

  data = g_strndup (text, str_len);
  data = g_realloc (data, str_len + 1 + 2 * sizeof (gint));

  *(gint *) (data + str_len + 1) = len;
  *(gint *) (data + str_len + 1 + sizeof (gint)) = cursor_index;

  aim_send_message (socket, AIM_MESSAGE_SET_SURROUNDING, data,
                    str_len + 1 + 2 * sizeof (gint), g_free);
  aim_result_iteration_until (im->result, aim_im_sockets_context,
                              AIM_MESSAGE_SET_SURROUNDING_REPLY);
}

void aim_im_focus_in (AimIM *im)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_if_fail (AIM_IS_IM (im));

  GSocket *socket = g_socket_connection_get_socket (im->connection);
  if (!socket || g_socket_is_closed (socket))
  {
    g_warning ("socket is closed");
    return;
  }

  aim_send_message (socket, AIM_MESSAGE_FOCUS_IN, NULL, 0, NULL);
  aim_result_iteration_until (im->result, aim_im_sockets_context,
                              AIM_MESSAGE_FOCUS_IN_REPLY);
}

void
aim_im_get_preedit_string (AimIM  *im,
                           gchar **str,
                           gint   *cursor_pos)
{
  g_debug (G_STRLOC ":%s", G_STRFUNC);

  g_return_if_fail (AIM_IS_IM (im));

  if (str)
    *str = g_strdup (im->preedit_string);

  if (cursor_pos)
    *cursor_pos = im->cursor_pos;
}

void aim_im_reset (AimIM *im)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_if_fail (AIM_IS_IM (im));

  GSocket *socket = g_socket_connection_get_socket (im->connection);
  if (!socket || g_socket_is_closed (socket))
  {
    g_warning ("socket is closed");
    return;
  }

  aim_send_message (socket, AIM_MESSAGE_RESET, NULL, 0, NULL);
  aim_result_iteration_until (im->result, aim_im_sockets_context,
                              AIM_MESSAGE_RESET_REPLY);
}

/* TODO: reduce duplicate code
 * aim_im_filter_event_fallback() is made from
 * aim_english_filter_event (AimEngine     *engine,
 *                           AimConnection *target,
 *                           AimEvent      *event);
 */
gboolean
aim_im_filter_event_fallback (AimIM    *im,
                              AimEvent *event)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  gboolean retval = FALSE;

  if ((event->key.type   == AIM_EVENT_KEY_RELEASE) ||
      (event->key.keyval == AIM_KEY_Shift_L)       ||
      (event->key.keyval == AIM_KEY_Shift_R)       ||
      (event->key.state & (AIM_CONTROL_MASK | AIM_MOD1_MASK)))
    return FALSE;

  gchar c = 0;

  if (event->key.keyval >= 32 && event->key.keyval <= 126)
    c = event->key.keyval;

  if (!c)
  {
    switch (event->key.keyval)
    {
      case AIM_KEY_KP_Multiply: c = '*'; break;
      case AIM_KEY_KP_Add:      c = '+'; break;
      case AIM_KEY_KP_Subtract: c = '-'; break;
      case AIM_KEY_KP_Divide:   c = '/'; break;
      default:
        break;
    }
  }

  if (!c && (event->key.state & AIM_MOD2_MASK))
  {
    switch (event->key.keyval)
    {
      case AIM_KEY_KP_Decimal:  c = '.'; break;
      case AIM_KEY_KP_0:        c = '0'; break;
      case AIM_KEY_KP_1:        c = '1'; break;
      case AIM_KEY_KP_2:        c = '2'; break;
      case AIM_KEY_KP_3:        c = '3'; break;
      case AIM_KEY_KP_4:        c = '4'; break;
      case AIM_KEY_KP_5:        c = '5'; break;
      case AIM_KEY_KP_6:        c = '6'; break;
      case AIM_KEY_KP_7:        c = '7'; break;
      case AIM_KEY_KP_8:        c = '8'; break;
      case AIM_KEY_KP_9:        c = '9'; break;
      default:
        break;
    }
  }

  if (c)
  {
    gchar *str = g_strdup_printf ("%c", c);
    g_signal_emit_by_name (im, "commit", str);
    g_free (str);
    retval = TRUE;
  }

  return retval;
}

gboolean aim_im_filter_event (AimIM *im, AimEvent *event)
{
  g_debug (G_STRLOC ":%s", G_STRFUNC);

  g_return_val_if_fail (AIM_IS_IM (im), FALSE);

  GSocket *socket = g_socket_connection_get_socket (im->connection);
  if (!socket || g_socket_is_closed (socket))
  {
    g_warning ("socket is closed");
    return aim_im_filter_event_fallback (im, event);
  }

  aim_send_message (socket, AIM_MESSAGE_FILTER_EVENT, event,
                    sizeof (AimEvent), NULL);
  aim_result_iteration_until (im->result, aim_im_sockets_context,
                              AIM_MESSAGE_FILTER_EVENT_REPLY);

  if (im->result->reply == NULL)
    return aim_im_filter_event_fallback (im, event);

  return *(gboolean *) (im->result->reply->data);
}

AimIM *
aim_im_new (void)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return g_object_new (AIM_TYPE_IM, NULL);
}

static void
aim_im_init (AimIM *im)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GSocketClient  *client;
  GSocketAddress *address;
  GSocket        *socket;
  GError         *error = NULL;

  im->preedit_string = g_strdup ("");

  address = g_unix_socket_address_new_with_type (AIM_ADDRESS, -1,
                                                 G_UNIX_SOCKET_ADDRESS_ABSTRACT);
  client = g_socket_client_new ();
  im->connection = g_socket_client_connect (client,
                                            G_SOCKET_CONNECTABLE (address),
                                            NULL, &error);
  g_object_unref (address);
  g_object_unref (client);

  if (im->connection == NULL)
  {
    g_critical (G_STRLOC ": %s: %s", G_STRFUNC, error->message);
    g_clear_error (&error);
    return;
  }

  socket = g_socket_connection_get_socket (im->connection);

  if (!socket)
  {
    g_critical (G_STRLOC ": %s: %s", G_STRFUNC, "Can't get socket");
    return;
  }

  AimMessage *message;

  AimConnectionType type = AIM_CONNECTION_AIM_IM;

  aim_send_message (socket, AIM_MESSAGE_CONNECT, &type, sizeof (AimConnectionType), NULL);
  g_socket_condition_wait (socket, G_IO_IN, NULL, NULL);
  message = aim_recv_message (socket);

  if (G_UNLIKELY (message == NULL ||
                  message->header->type != AIM_MESSAGE_CONNECT_REPLY))
  {
    aim_message_unref (message);
    g_error ("Couldn't connect to aim daemon");
  }

  aim_message_unref (message);

  im->result = g_slice_new0 (AimResult);

  GMutex mutex;

  g_mutex_init (&mutex);
  g_mutex_lock (&mutex);

  if (G_UNLIKELY (aim_im_sockets_context == NULL))
  {
    aim_im_sockets_context = g_main_context_new ();
    aim_im_sockets_context_ref_count++;
  }
  else
  {
    aim_im_sockets_context = g_main_context_ref (aim_im_sockets_context);
    aim_im_sockets_context_ref_count++;
  }

  g_mutex_unlock (&mutex);

  /* when g_main_context_iteration(), iterate only sockets */
  im->sockets_context_source = g_socket_create_source (socket, G_IO_IN, NULL);
  g_source_set_can_recurse (im->sockets_context_source, TRUE);
  g_source_attach (im->sockets_context_source, aim_im_sockets_context);
  g_source_set_callback (im->sockets_context_source,
                         (GSourceFunc) on_incoming_message,
                         im, NULL);

  im->default_context_source = g_socket_create_source (socket, G_IO_IN, NULL);
  g_source_set_can_recurse (im->default_context_source, TRUE);
  g_source_set_callback (im->default_context_source,
                         (GSourceFunc) on_incoming_message, im, NULL);
  g_source_attach (im->default_context_source, NULL);
}

static void
aim_im_finalize (GObject *object)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  AimIM *im = AIM_IM (object);

  if (im->sockets_context_source)
  {
    g_source_destroy (im->sockets_context_source);
    g_source_unref   (im->sockets_context_source);
  }

  if (im->default_context_source)
  {
    g_source_destroy (im->default_context_source);
    g_source_unref   (im->default_context_source);
  }

  if (im->connection)
    g_object_unref (im->connection);

  GMutex mutex;

  g_mutex_init (&mutex);
  g_mutex_lock (&mutex);

  if (aim_im_sockets_context)
  {
    g_main_context_unref (aim_im_sockets_context);
    aim_im_sockets_context_ref_count--;

    if (aim_im_sockets_context_ref_count == 0)
      aim_im_sockets_context = NULL;
  }

  g_mutex_unlock (&mutex);
  g_free (im->preedit_string);

  if (im->result)
    g_slice_free (AimResult, im->result);

  G_OBJECT_CLASS (aim_im_parent_class)->finalize (object);
}

static void
aim_im_class_init (AimIMClass *klass)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = aim_im_finalize;

  im_signals[PREEDIT_START] =
    g_signal_new (g_intern_static_string ("preedit-start"),
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (AimIMClass, preedit_start),
                  NULL, NULL,
                  aim_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  im_signals[PREEDIT_END] =
    g_signal_new (g_intern_static_string ("preedit-end"),
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (AimIMClass, preedit_end),
                  NULL, NULL,
                  aim_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  im_signals[PREEDIT_CHANGED] =
    g_signal_new (g_intern_static_string ("preedit-changed"),
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (AimIMClass, preedit_changed),
                  NULL, NULL,
                  aim_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  im_signals[COMMIT] =
    g_signal_new (g_intern_static_string ("commit"),
                  G_OBJECT_CLASS_TYPE (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (AimIMClass, commit),
                  NULL, NULL,
                  aim_cclosure_marshal_VOID__STRING,
                  G_TYPE_NONE, 1,
                  G_TYPE_STRING);

  im_signals[RETRIEVE_SURROUNDING] =
    g_signal_new (g_intern_static_string ("retrieve-surrounding"),
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (AimIMClass, retrieve_surrounding),
                  g_signal_accumulator_true_handled, NULL,
                  aim_cclosure_marshal_BOOLEAN__VOID,
                  G_TYPE_BOOLEAN, 0);

  im_signals[DELETE_SURROUNDING] =
    g_signal_new (g_intern_static_string ("delete-surrounding"),
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (AimIMClass, delete_surrounding),
                  g_signal_accumulator_true_handled, NULL,
                  aim_cclosure_marshal_BOOLEAN__INT_INT,
                  G_TYPE_BOOLEAN, 2,
                  G_TYPE_INT,
                  G_TYPE_INT);
}
