#include "ats_control.h"

#include <string.h>

static gboolean
incoming_callback  (GSocketService *service,
                    GSocketConnection *connection,
                    GObject *source_object,
                    gpointer user_data)
{
  GInputStream * istream = g_io_stream_get_input_stream (G_IO_STREAM (connection));
  gchar message[1024];
  g_input_stream_read  (istream,
                        message,
                        1024,
                        NULL,
                        NULL);
  return FALSE;
}

ATS_CONTROL*
ats_control_new(ATS_TREE* tree)
{
  ATS_CONTROL* rval = g_new(ATS_CONTROL, 1);
  rval->incoming_service = g_socket_service_new ();
  g_socket_listener_add_inet_port ((GSocketListener*)rval->incoming_service,
                                    1500, /* your port goes here */
                                    NULL,
                                    NULL);
  g_signal_connect (rval->incoming_service,
                    "incoming",
                    G_CALLBACK (incoming_callback),
                    NULL);
  g_socket_service_start (rval->incoming_service);
  rval->client = g_socket_client_new();
  return rval;
}

void
ats_control_delete(ATS_CONTROL* this)
{
  g_free(this);
}

void
ats_control_send(ATS_CONTROL* this, gchar* message)
{
  //GSocketClient * client = g_socket_client_new();
  GSocketConnection* connection = g_socket_client_connect_to_host (this->client,
								  (gchar*)"localhost",
								  1600, /* your port goes here */
								  NULL,
								  NULL);
  GOutputStream * ostream = g_io_stream_get_output_stream (G_IO_STREAM (connection));
  g_output_stream_write  (ostream,
			  message,
                          strlen(message),
                          NULL,
                          NULL);
  g_io_stream_close(G_IO_STREAM (connection), NULL, NULL);
}
