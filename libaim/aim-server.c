/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*-  */
/*
 * aim-server.c
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

#include "config.h"
#include "aim-server.h"
#include "aim-private.h"
#include <string.h>
#include <gio/gunixsocketaddress.h>
#include "aim-module.h"
#include "IMdkit/Xi18n.h"
#include "aim-english.h"

enum
{
  PROP_0,
  PROP_ADDRESS,
};

static gboolean
on_incoming_message_aim (GSocket       *socket,
                         GIOCondition   condition,
                         AimConnection *connection)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  AimMessage *message;
  gboolean    retval;
  aim_message_unref (connection->result->reply);
  connection->result->is_dispatched = TRUE;

  if (condition & (G_IO_HUP | G_IO_ERR))
  {
    g_debug (G_STRLOC ": condition & (G_IO_HUP | G_IO_ERR)");

    g_socket_close (socket, NULL);

    connection->result->reply   = NULL;

    if (G_UNLIKELY (connection->type == AIM_CONNECTION_AIM_AGENT))
      connection->server->agents_list =
        g_list_remove (connection->server->agents_list, connection);

    g_hash_table_remove (connection->server->connections,
                         GUINT_TO_POINTER (aim_connection_get_id (connection)));

    return G_SOURCE_REMOVE;
  }

  if (connection->type == AIM_CONNECTION_AIM_IM)
    aim_engine_set_english_mode (connection->engine,
                                 connection->is_english_mode);

  message = aim_recv_message (socket);
  connection->result->reply = message;

  if (G_UNLIKELY (message == NULL))
  {
    g_critical (G_STRLOC ": NULL message");
    return G_SOURCE_CONTINUE;
  }

  switch (message->header->type)
  {
    case AIM_MESSAGE_FILTER_EVENT:
      aim_message_ref (message);
      retval = aim_connection_filter_event (connection,
                                            (AimEvent *) message->data);
      aim_message_unref (message);
      aim_send_message (socket, AIM_MESSAGE_FILTER_EVENT_REPLY, &retval,
                        sizeof (gboolean), NULL);
      break;
    case AIM_MESSAGE_RESET:
      aim_connection_reset (connection);
      aim_send_message (socket, AIM_MESSAGE_RESET_REPLY, NULL, 0, NULL);
      break;
    case AIM_MESSAGE_FOCUS_IN:
      aim_connection_focus_in (connection);
      aim_send_message (socket, AIM_MESSAGE_FOCUS_IN_REPLY, NULL, 0, NULL);
      break;
    case AIM_MESSAGE_FOCUS_OUT:
      aim_connection_focus_out (connection);
      aim_send_message (socket, AIM_MESSAGE_FOCUS_OUT_REPLY, NULL, 0, NULL);
      break;
    case AIM_MESSAGE_SET_SURROUNDING:
      {
        aim_message_ref (message);
        gchar   *data     = message->data;
        guint16  data_len = message->header->data_len;

        gint   str_len      = data_len - 1 - 2 * sizeof (gint);
        gint   cursor_index = *(gint *) (data + data_len - sizeof (gint));

        aim_connection_set_surrounding (connection, data, str_len,
                                        cursor_index);
        aim_message_unref (message);
        aim_send_message (socket, AIM_MESSAGE_SET_SURROUNDING_REPLY, NULL, 0, NULL);
      }
      break;
    case AIM_MESSAGE_GET_SURROUNDING:
      {
        gchar *data;
        gint   cursor_index;
        gint   str_len = 0;

        retval = aim_connection_get_surrounding (connection, &data,
                                                 &cursor_index);
        str_len = strlen (data);
        data = g_realloc (data, str_len + 1 + sizeof (gint) + sizeof (gboolean));
        *(gint *) (data + str_len + 1) = cursor_index;
        *(gboolean *) (data + str_len + 1 + sizeof (gint)) = retval;

        aim_send_message (socket, AIM_MESSAGE_GET_SURROUNDING_REPLY,
                          data,
                          str_len + 1 + sizeof (gint) + sizeof (gboolean),
                          NULL);
        g_free (data);
      }
      break;
    case AIM_MESSAGE_SET_CURSOR_LOCATION:
      aim_message_ref (message);
      aim_connection_set_cursor_location (connection,
                                          (AimRectangle *) message->data);
      aim_message_unref (message);
      aim_send_message (socket, AIM_MESSAGE_SET_CURSOR_LOCATION_REPLY,
                        NULL, 0, NULL);
      break;
    case AIM_MESSAGE_SET_USE_PREEDIT:
      aim_message_ref (message);
      aim_connection_set_use_preedit (connection, *(gboolean *) message->data);
      aim_message_unref (message);
      aim_send_message (socket, AIM_MESSAGE_SET_USE_PREEDIT_REPLY,
                        NULL, 0, NULL);
      break;
    case AIM_MESSAGE_PREEDIT_START_REPLY:
    case AIM_MESSAGE_PREEDIT_CHANGED_REPLY:
    case AIM_MESSAGE_PREEDIT_END_REPLY:
    case AIM_MESSAGE_COMMIT_REPLY:
    case AIM_MESSAGE_RETRIEVE_SURROUNDING_REPLY:
    case AIM_MESSAGE_DELETE_SURROUNDING_REPLY:
      break;
    default:
      g_warning ("Unknown message type: %d", message->header->type);
      break;
  }

  if (connection->type == AIM_CONNECTION_AIM_IM)
    connection->is_english_mode =
      aim_engine_get_english_mode (connection->engine);

  return G_SOURCE_CONTINUE;
}

static guint16
aim_server_add_connection (AimServer     *server,
                           AimConnection *connection)
{
  guint16 id;

  do
    id = server->next_id++;
  while (id == 0 || g_hash_table_contains (server->connections,
                                           GUINT_TO_POINTER (id)));
  connection->id = id;
  connection->server = server;
  g_hash_table_insert (server->connections, GUINT_TO_POINTER (id), connection);

  return id;
}

static gboolean
on_new_connection (GSocketService    *service,
                   GSocketConnection *socket_connection,
                   GObject           *source_object,
                   AimServer         *server)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GSocket *socket = g_socket_connection_get_socket (socket_connection);

  AimMessage *message;
  message = aim_recv_message (socket);

  if (G_UNLIKELY (message == NULL ||
                  message->header->type != AIM_MESSAGE_CONNECT))
  {
    g_critical (G_STRLOC ": Couldn't connect");
    aim_message_unref (message);
    aim_send_message (socket, AIM_MESSAGE_ERROR, NULL, 0, NULL);
    return FALSE;
  }

  aim_send_message (socket, AIM_MESSAGE_CONNECT_REPLY, NULL, 0, NULL);

  AimConnection *connection;
  connection = aim_connection_new (*(AimConnectionType *) message->data,
                                   aim_server_get_default_engine (server), NULL);
  aim_message_unref (message);
  connection->socket = socket;
  aim_server_add_connection (server, connection);

  if (connection->type == AIM_CONNECTION_AIM_AGENT)
    server->agents_list = g_list_prepend (server->agents_list, connection);

  connection->source = g_socket_create_source (socket, G_IO_IN, NULL);
  connection->socket_connection = g_object_ref (socket_connection);
  g_source_set_can_recurse (connection->source, TRUE);
  g_source_set_callback (connection->source,
                         (GSourceFunc) on_incoming_message_aim,
                         connection, NULL);
  g_source_attach (connection->source, server->main_context);

  return TRUE;
}

static gboolean
aim_server_initable_init (GInitable     *initable,
                          GCancellable  *cancellable,
                          GError       **error)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  AimServer      *server = AIM_SERVER (initable);
  GSocketAddress *address;
  GError         *local_error = NULL;

  server->listener = G_SOCKET_LISTENER (g_socket_service_new ());
  /* server->listener = G_SOCKET_LISTENER (g_threaded_socket_service_new (-1)); */

  if (g_unix_socket_address_abstract_names_supported ())
    address = g_unix_socket_address_new_with_type (server->address, -1,
                                                   G_UNIX_SOCKET_ADDRESS_ABSTRACT);
  else
  {
    g_set_error_literal (error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED,
                         "Abstract UNIX domain socket names are not supported.");
    return FALSE;
  }

  g_socket_listener_add_address (server->listener, address,
                                 G_SOCKET_TYPE_STREAM,
                                 G_SOCKET_PROTOCOL_DEFAULT,
                                 NULL, NULL, &local_error);
  g_object_unref (address);

  if (local_error)
  {
    g_propagate_error (error, local_error);
    return FALSE;
  }

  server->is_using_listener = TRUE;
  server->run_signal_handler_id =
    g_signal_connect (G_SOCKET_SERVICE (server->listener), "incoming",
                      (GCallback) on_new_connection, server);
/*
  server->run_signal_handler_id = g_signal_connect (G_SOCKET_SERVICE (server->listener),
                                                    "run",
                                                    G_CALLBACK (on_run),
                                                    server);
*/

  return TRUE;
}

static void
aim_server_initable_iface_init (GInitableIface *initable_iface)
{
  initable_iface->init = aim_server_initable_init;
}

G_DEFINE_TYPE_WITH_CODE (AimServer, aim_server, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (G_TYPE_INITABLE,
                                                aim_server_initable_iface_init));

static gint
on_comparing_engine_with_id (AimEngine *engine, const gchar *id)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  return g_strcmp0 (aim_engine_get_id (engine), id);
}

AimEngine *
aim_server_get_instance (AimServer   *server,
                         const gchar *id)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GList *list;

  list = g_list_find_custom (g_list_first (server->instances),
                             id,
                             (GCompareFunc) on_comparing_engine_with_id);
  if (list)
    return list->data;

  return NULL;
}

AimEngine *
aim_server_get_next_instance (AimServer *server, AimEngine *engine)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GList *list;

  server->instances = g_list_first (server->instances);
  server->instances = g_list_find  (server->instances, engine);

  list = g_list_next (server->instances);

  if (list == NULL)
    list = g_list_first (server->instances);

  if (list)
  {
    server->instances = list;
    return list->data;
  }

  g_assert (list != NULL);

  return engine;
}

AimEngine *
aim_server_get_default_engine (AimServer *server)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GSettings *settings = g_settings_new ("org.aim");
  gchar *engine_id = g_settings_get_string (settings, "default-engine");
  AimEngine *engine = aim_server_get_instance (server, engine_id);

  g_free (engine_id);
  g_object_unref (settings);

  if (engine == NULL)
    engine = aim_server_get_instance (server, "aim-english");

  g_assert (engine != NULL);

  return engine;
}

static GList *aim_server_create_module_instances (AimServer *server)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GList *instances = NULL;

  GHashTableIter iter;
  gpointer value;

  g_hash_table_iter_init (&iter, server->module_manager->modules);
  while (g_hash_table_iter_next (&iter, NULL, &value))
  {
    AimModule *module = value;
    instances = g_list_prepend (instances, g_object_new (module->type,
                                                          "server", server,
                                                          NULL));
  }

  /* add english engine */
  instances = g_list_prepend (instances,
                              g_object_new (AIM_TYPE_ENGLISH,
                                            "server", server, NULL));
  return instances;
}

static void
aim_server_init (AimServer *server)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GSettings *settings = g_settings_new ("org.aim");
  gchar **hotkeys = g_settings_get_strv (settings, "hotkeys");
  server->hotkeys = aim_key_newv ((const gchar **) hotkeys);
  g_object_unref (settings);
  g_strfreev (hotkeys);

  server->candidate = aim_candidate_new ();
  server->module_manager = aim_module_manager_get_default ();
  server->instances = aim_server_create_module_instances (server);

  server->main_context = g_main_context_ref_thread_default ();
  server->connections = g_hash_table_new_full (g_direct_hash,
                                               g_direct_equal,
                                               NULL,
                                               (GDestroyNotify) g_object_unref);
  server->agents_list = NULL;
}

void
aim_server_stop (AimServer *server)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_if_fail (AIM_IS_SERVER (server));

  if (!server->active)
    return;

  g_assert (server->is_using_listener);
  g_assert (server->run_signal_handler_id > 0);

  g_signal_handler_disconnect (server->listener, server->run_signal_handler_id);
  server->run_signal_handler_id = 0;

  g_socket_service_stop (G_SOCKET_SERVICE (server->listener));
  server->active = FALSE;
}

static void
aim_server_finalize (GObject *object)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  AimServer *server = AIM_SERVER (object);

  if (server->run_signal_handler_id > 0)
    g_signal_handler_disconnect (server->listener, server->run_signal_handler_id);

  if (server->listener != NULL)
    g_object_unref (server->listener);

  g_object_unref (server->module_manager);

  if (server->instances)
  {
    g_list_free_full (server->instances, g_object_unref);
    server->instances = NULL;
  }

  g_object_unref (server->candidate);
  g_hash_table_unref (server->connections);
  g_list_free (server->agents_list);
  aim_key_freev (server->hotkeys);
  g_free (server->address);

  if (server->xevent_source)
  {
    g_source_destroy (server->xevent_source);
    g_source_unref   (server->xevent_source);
  }

  g_main_context_unref (server->main_context);

  G_OBJECT_CLASS (aim_server_parent_class)->finalize (object);
}

static void
aim_server_get_property (GObject    *object,
                         guint       prop_id,
                         GValue     *value,
                         GParamSpec *pspec)
{
  AimServer *server = AIM_SERVER (object);

  switch (prop_id)
  {
    case PROP_ADDRESS:
      g_value_set_string (value, server->address);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
aim_server_set_property (GObject      *object,
                         guint         prop_id,
                         const GValue *value,
                         GParamSpec   *pspec)
{
  AimServer *server = AIM_SERVER (object);

  switch (prop_id)
  {
    case PROP_ADDRESS:
      server->address = g_value_dup_string (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
aim_server_class_init (AimServerClass *class)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GObjectClass *object_class = G_OBJECT_CLASS (class);

  object_class->finalize     = aim_server_finalize;
  object_class->set_property = aim_server_set_property;
  object_class->get_property = aim_server_get_property;

  g_object_class_install_property (object_class,
                                   PROP_ADDRESS,
                                   g_param_spec_string ("address",
                                                        "Address",
                                                        "The address to listen on",
                                                        NULL,
                                                        G_PARAM_READABLE |
                                                        G_PARAM_WRITABLE |
                                                        G_PARAM_CONSTRUCT_ONLY |
                                                        G_PARAM_STATIC_NAME |
                                                        G_PARAM_STATIC_BLURB |
                                                        G_PARAM_STATIC_NICK));
}

AimServer *
aim_server_new (const gchar  *address,
                GError      **error)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_val_if_fail (address != NULL, NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  return g_initable_new (AIM_TYPE_SERVER, NULL, error,
                         "address", address, NULL);
}

typedef struct
{
  GSource  source;
  Display *display;
  GPollFD  poll_fd;
} AimXEventSource;

static gboolean aim_xevent_source_prepare (GSource *source,
                                           gint    *timeout)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  Display *display = ((AimXEventSource *) source)->display;
  *timeout = -1;
  return XPending (display) > 0;
}

static gboolean aim_xevent_source_check (GSource *source)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  AimXEventSource *display_source = (AimXEventSource *) source;

  if (display_source->poll_fd.revents & G_IO_IN)
    return XPending (display_source->display) > 0;
  else
    return FALSE;
}

int aim_server_xim_set_ic_values (AimServer        *server,
                                  XIMS              xims,
                                  IMChangeICStruct *data)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  AimConnection *connection;
  connection = g_hash_table_lookup (server->connections,
                                    GUINT_TO_POINTER (data->icid));
  CARD16 i;

  for (i = 0; i < data->ic_attr_num; i++)
  {
    if (g_strcmp0 (XNInputStyle, data->ic_attr[i].name) == 0)
      g_message ("XNInputStyle is ignored");
    else if (g_strcmp0 (XNClientWindow, data->ic_attr[i].name) == 0)
      connection->client_window = *(Window *) data->ic_attr[i].value;
    else if (g_strcmp0 (XNFocusWindow, data->ic_attr[i].name) == 0)
      connection->focus_window = *(Window *) data->ic_attr[i].value;
    else
      g_warning (G_STRLOC ": %s %s", G_STRFUNC, data->ic_attr[i].name);
  }

  for (i = 0; i < data->preedit_attr_num; i++)
  {
    if (g_strcmp0 (XNPreeditState, data->preedit_attr[i].name) == 0)
    {
      XIMPreeditState state = *(XIMPreeditState *) data->preedit_attr[i].value;
      switch (state)
      {
        case XIMPreeditEnable:
          aim_connection_set_use_preedit (connection, TRUE);
          break;
        case XIMPreeditDisable:
          aim_connection_set_use_preedit (connection, FALSE);
          break;
        default:
          g_message ("XIMPreeditState: %ld is ignored", state);
          break;
      }
    }
    else
      g_critical (G_STRLOC ": %s: %s is ignored",
                  G_STRFUNC, data->preedit_attr[i].name);
  }

  for (i = 0; i < data->status_attr_num; i++)
  {
    g_critical (G_STRLOC ": %s: %s is ignored",
                G_STRFUNC, data->status_attr[i].name);
  }

  aim_connection_xim_set_cursor_location (connection, xims->core.display);

  return 1;
}

int aim_server_xim_create_ic (AimServer        *server,
                              XIMS              xims,
                              IMChangeICStruct *data)
{
  g_debug (G_STRLOC ": %s, data->connect_id: %d", G_STRFUNC, data->connect_id);

  AimConnection *connection;
  connection = g_hash_table_lookup (server->connections,
                                    GUINT_TO_POINTER ((gint) data->icid));

  if (!connection)
  {
    connection = aim_connection_new (AIM_CONNECTION_XIM,
                                     aim_server_get_default_engine (server),
                                     xims);
    connection->xim_connect_id = data->connect_id;
    data->icid = aim_server_add_connection (server, connection);
    g_debug (G_STRLOC ": icid = %d", data->icid);
  }

  aim_server_xim_set_ic_values (server, xims, data);

  return 1;
}

int aim_server_xim_destroy_ic (AimServer         *server,
                               XIMS               xims,
                               IMDestroyICStruct *data)
{
  g_debug (G_STRLOC ": %s, data->icid = %d", G_STRFUNC, data->icid);

  return g_hash_table_remove (server->connections,
                              GUINT_TO_POINTER (data->icid));
}

int aim_server_xim_get_ic_values (AimServer        *server,
                                  XIMS              xims,
                                  IMChangeICStruct *data)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  AimConnection *connection;
  connection = g_hash_table_lookup (server->connections,
                                    GUINT_TO_POINTER (data->icid));
  CARD16 i;

  for (i = 0; i < data->ic_attr_num; i++)
  {
    if (g_strcmp0 (XNFilterEvents, data->ic_attr[i].name) == 0)
    {
      data->ic_attr[i].value_length = sizeof (CARD32);
      data->ic_attr[i].value = g_malloc (sizeof (CARD32));
      *(CARD32 *) data->ic_attr[i].value = KeyPressMask | KeyReleaseMask;
    }
    else if (g_strcmp0 (XNSeparatorofNestedList, data->ic_attr[i].name) == 0)
    {
      data->ic_attr[i].value_length = sizeof (CARD16);
      data->ic_attr[i].value = g_malloc (sizeof (CARD16));
      *(CARD16 *) data->ic_attr[i].value = 0;
    }
    else
      g_critical (G_STRLOC ": %s: %s is ignored",
                  G_STRFUNC, data->ic_attr[i].name);
  }

  for (i = 0; i < data->preedit_attr_num; i++)
  {
    if (g_strcmp0 (XNPreeditState, data->preedit_attr[i].name) == 0)
    {
      data->preedit_attr[i].value_length = sizeof (XIMPreeditState);
      data->preedit_attr[i].value = g_malloc (sizeof (XIMPreeditState));

      if (connection->use_preedit)
        *(XIMPreeditState *) data->preedit_attr[i].value = XIMPreeditEnable;
      else
        *(XIMPreeditState *) data->preedit_attr[i].value = XIMPreeditDisable;
    }
    else
      g_critical (G_STRLOC ": %s: %s is ignored",
                  G_STRFUNC, data->preedit_attr[i].name);
  }

  for (i = 0; i < data->status_attr_num; i++)
    g_critical (G_STRLOC ": %s: %s is ignored",
                G_STRFUNC, data->status_attr[i].name);

  return 1;
}

int aim_server_xim_set_ic_focus (AimServer           *server,
                                 XIMS                 xims,
                                 IMChangeFocusStruct *data)
{
  AimConnection *connection;
  connection = g_hash_table_lookup (server->connections,
                                    GUINT_TO_POINTER (data->icid));

  g_debug (G_STRLOC ": %s, icid = %d, connection id = %d",
           G_STRFUNC, data->icid, connection->id);

  aim_connection_focus_in (connection);

  return 1;
}

int aim_server_xim_unset_ic_focus (AimServer           *server,
                                   XIMS                 xims,
                                   IMChangeFocusStruct *data)
{
  AimConnection *connection;
  connection = g_hash_table_lookup (server->connections,
                                    GUINT_TO_POINTER (data->icid));

  g_debug (G_STRLOC ": %s, icid = %d, connection id = %d",
           G_STRFUNC, data->icid, connection->id);

  aim_connection_focus_out (connection);

  return 1;
}

int aim_server_xim_forward_event (AimServer            *server,
                                  XIMS                  xims,
                                  IMForwardEventStruct *data)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  XKeyEvent    *xevent;
  AimEvent     *event;
  AimEventType  type;
  gboolean      retval;

  xevent = (XKeyEvent*) &(data->event);

  type = (xevent->type == KeyPress) ? AIM_EVENT_KEY_PRESS : AIM_EVENT_KEY_RELEASE;

  event = aim_event_new (type);
  event->key.state = (AimModifierType) xevent->state;
  event->key.keyval = XLookupKeysym (xevent,
                                     (!(xevent->state & ShiftMask) !=
                                      !(xevent->state & LockMask)) ? 1 : 0);
  event->key.hardware_keycode = xevent->keycode;

  AimConnection *connection;
  connection = g_hash_table_lookup (server->connections,
                                    GUINT_TO_POINTER (data->icid));
  retval = aim_connection_filter_event (connection, event);
  aim_event_free (event);

  if (G_UNLIKELY (!retval))
    IMForwardEvent (xims, (XPointer) data);

  return 1;
}

int aim_server_xim_reset_ic (AimServer       *server,
                             XIMS             xims,
                             IMResetICStruct *data)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  AimConnection *connection;
  connection = g_hash_table_lookup (server->connections,
                                    GUINT_TO_POINTER (data->icid));
  aim_connection_reset (connection);

  return 1;
}

static int
on_incoming_message_xim (XIMS        xims,
                         IMProtocol *data,
                         AimServer  *server)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_val_if_fail (xims != NULL, True);
  g_return_val_if_fail (data != NULL, True);

  if (!AIM_IS_SERVER (server))
    g_error ("ERROR: IMUserData");

  int retval;

  AimConnection *connection = NULL;

  if (data->major_code == XIM_CREATE_IC      ||
      data->major_code == XIM_DESTROY_IC     ||
      data->major_code == XIM_SET_IC_VALUES  ||
      data->major_code == XIM_GET_IC_VALUES  ||
      data->major_code == XIM_FORWARD_EVENT  ||
      data->major_code == XIM_SET_IC_FOCUS   ||
      data->major_code == XIM_UNSET_IC_FOCUS ||
      data->major_code == XIM_RESET_IC)
  {
    connection = g_hash_table_lookup (server->connections,
                                      GUINT_TO_POINTER (data->changeic.icid));
    if (connection)
      aim_engine_set_english_mode (connection->engine,
                                   connection->is_english_mode);
  }

  switch (data->major_code)
  {
    case XIM_OPEN:
      g_debug (G_STRLOC ": XIM_OPEN: connect_id: %u", data->imopen.connect_id);
      retval = 1;
      break;
    case XIM_CLOSE:
      g_debug (G_STRLOC ": XIM_CLOSE: connect_id: %u",
               data->imclose.connect_id);
      retval = 1;
      break;
    case XIM_PREEDIT_START_REPLY:
      g_debug (G_STRLOC ": XIM_PREEDIT_START_REPLY");
      retval = 1;
      break;
    case XIM_CREATE_IC:
      retval = aim_server_xim_create_ic (server, xims, &data->changeic);
      break;
    case XIM_DESTROY_IC:
      retval = aim_server_xim_destroy_ic (server, xims, &data->destroyic);
      break;
    case XIM_SET_IC_VALUES:
      retval = aim_server_xim_set_ic_values (server, xims, &data->changeic);
      break;
    case XIM_GET_IC_VALUES:
      retval = aim_server_xim_get_ic_values (server, xims, &data->changeic);
      break;
    case XIM_FORWARD_EVENT:
      retval = aim_server_xim_forward_event (server, xims, &data->forwardevent);
      break;
    case XIM_SET_IC_FOCUS:
      retval = aim_server_xim_set_ic_focus (server, xims, &data->changefocus);
      break;
    case XIM_UNSET_IC_FOCUS:
      retval = aim_server_xim_unset_ic_focus (server, xims, &data->changefocus);
      break;
    case XIM_RESET_IC:
      retval = aim_server_xim_reset_ic (server, xims, &data->resetic);
      break;
    default:
      g_warning (G_STRLOC ": %s: major op code %d not handled", G_STRFUNC,
                 data->major_code);
      retval = 0;
      break;
  }

  if (connection)
    connection->is_english_mode =
      aim_engine_get_english_mode (connection->engine);

  return retval;
}

static gboolean aim_xevent_source_dispatch (GSource     *source,
                                            GSourceFunc  callback,
                                            gpointer     user_data)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  Display *display = ((AimXEventSource*) source)->display;
  XEvent   event;

  while (XPending (display))
  {
    XNextEvent (display, &event);
    if (XFilterEvent (&event, None))
      continue;
  }

  return TRUE;
}

static void aim_xevent_source_finalize (GSource *source)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);
}

static GSourceFuncs event_funcs = {
  aim_xevent_source_prepare,
  aim_xevent_source_check,
  aim_xevent_source_dispatch,
  aim_xevent_source_finalize
};

GSource *
aim_xevent_source_new (Display *display)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  GSource *source;
  AimXEventSource *xevent_source;
  int connection_number;

  source = g_source_new (&event_funcs, sizeof (AimXEventSource));
  xevent_source = (AimXEventSource *) source;
  xevent_source->display = display;

  connection_number = ConnectionNumber (xevent_source->display);

  xevent_source->poll_fd.fd = connection_number;
  xevent_source->poll_fd.events = G_IO_IN;
  g_source_add_poll (source, &xevent_source->poll_fd);

  g_source_set_priority (source, G_PRIORITY_DEFAULT);
  g_source_set_can_recurse (source, FALSE);

  return source;
}

static int
on_xerror (Display *display, XErrorEvent *error)
{
  gchar err_msg[64];

  XGetErrorText (display, error->error_code, err_msg, 63);
  g_warning (G_STRLOC ": %s: XError: %s "
    "serial=%lu, error_code=%d request_code=%d minor_code=%d resourceid=%lu",
    G_STRFUNC, err_msg, error->serial, error->error_code, error->request_code,
    error->minor_code, error->resourceid);

  return 1;
}

static gboolean
aim_server_init_xims (AimServer *server)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  Display *display;
  Window   window;

  display = XOpenDisplay (NULL);

  if (display == NULL)
    return FALSE;

  XIMStyle ims_styles_on_spot [] = {
    XIMPreeditPosition  | XIMStatusNothing,
    XIMPreeditCallbacks | XIMStatusNothing,
    XIMPreeditNothing   | XIMStatusNothing,
    XIMPreeditPosition  | XIMStatusCallbacks,
    XIMPreeditCallbacks | XIMStatusCallbacks,
    XIMPreeditNothing   | XIMStatusCallbacks,
    0
  };

  XIMEncoding ims_encodings[] = {
      "COMPOUND_TEXT",
      NULL
  };

  XIMStyles styles;
  XIMEncodings encodings;

  styles.count_styles = sizeof (ims_styles_on_spot) / sizeof (XIMStyle) - 1;
  styles.supported_styles = ims_styles_on_spot;

  encodings.count_encodings = sizeof (ims_encodings) / sizeof (XIMEncoding) - 1;
  encodings.supported_encodings = ims_encodings;

  XSetWindowAttributes attrs;

  attrs.event_mask = KeyPressMask | KeyReleaseMask;
  attrs.override_redirect = True;

  window = XCreateWindow (display,      /* Display *display */
                          DefaultRootWindow (display),  /* Window parent */
                          0, 0,         /* int x, y */
                          1, 1,         /* unsigned int width, height */
                          0,            /* unsigned int border_width */
                          0,            /* int depth */
                          InputOutput,  /* unsigned int class */
                          CopyFromParent, /* Visual *visual */
                          CWOverrideRedirect | CWEventMask, /* unsigned long valuemask */
                          &attrs);      /* XSetWindowAttributes *attributes */

  IMOpenIM (display,
            IMModifiers,        "Xi18n",
            IMServerWindow,     window,
            IMServerName,       PACKAGE,
            IMLocale,           "C,en,ko",
            IMServerTransport,  "X/",
            IMInputStyles,      &styles,
            IMEncodingList,     &encodings,
            IMProtocolHandler,  on_incoming_message_xim,
            IMUserData,         server,
            IMFilterEventMask,  KeyPressMask | KeyReleaseMask,
            NULL);

  server->xevent_source = aim_xevent_source_new (display);
  g_source_attach (server->xevent_source, server->main_context);
  XSetErrorHandler (on_xerror);

  return TRUE;
}

void
aim_server_start (AimServer *server)
{
  g_debug (G_STRLOC ": %s", G_STRFUNC);

  g_return_if_fail (AIM_IS_SERVER (server));

  if (server->active)
    return;

  g_assert (server->is_using_listener);
  g_socket_service_start (G_SOCKET_SERVICE (server->listener));

  if (aim_server_init_xims (server) == FALSE)
    g_warning ("XIM server is not starded");

  server->active = TRUE;
}
