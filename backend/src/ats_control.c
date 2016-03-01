#include "ats_control.h"

#include <string.h>
#include <stdlib.h>

static gboolean
parse_tree_message(guint *message, ATS_TREE* tree)
{
  ATS_METADATA* data = tree->metadata;
  guint prognum = 0;
  ATS_PID_DATA* piddata;
  ATS_CH_DATA* progdata;
  
  if (tree->branches != NULL)
    ats_tree_remove_branches(tree);
  
  message += 3; /* skip stream id */
  while (*message != TREE_HEADER){
    if (*message == PROG_DIVIDER) {
      message++;
      prognum = *message;
      progdata = ats_metadata_find_channel(data, *message);
      progdata->to_be_analyzed = TRUE;
      message++;
      progdata->xid = *message;
      message++;
    }
    else {
      piddata = ats_metadata_find_pid(data, prognum, *message);
       if (piddata) 
	 piddata->to_be_analyzed = TRUE;
      message++;
    }
  }
  ats_metadata_print(data);
  ats_tree_add_branches(tree);
  return FALSE;
}

static gboolean
incoming_callback  (GSocketService *service,
                    GSocketConnection *connection,
                    GObject *source_object,
                    gpointer user_data)
{
  ATS_TREE* tree = (ATS_TREE*) user_data;
  GInputStream * istream = g_io_stream_get_input_stream (G_IO_STREAM (connection));
  guint buffer[1024];
  guint *message;
  g_input_stream_read  (istream,
                        buffer,
                        256,
                        NULL,
                        NULL);
  message = buffer;
  if(*message == TREE_HEADER) 
    return parse_tree_message(message, tree);
  g_printerr("Error: unknown message type!\n");
  return FALSE;
}

ATS_CONTROL*
ats_control_new(ATS_TREE* tree, guint stream_id)
{
  ATS_CONTROL* rval = g_new(ATS_CONTROL, 1);
  /* input */
  rval->incoming_service = g_socket_service_new ();
  g_socket_listener_add_inet_port ((GSocketListener*)rval->incoming_service,
                                    1500+stream_id, /* your port goes here */
                                    NULL,
                                    NULL);
  g_signal_connect (rval->incoming_service,
                    "incoming",
                    G_CALLBACK (incoming_callback),
                    tree);
  g_socket_service_start (rval->incoming_service);
  /* output */
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
  GSocketConnection* connection = g_socket_client_connect_to_host (this->client,
								   (gchar*)"localhost",
								   1600, /* your port goes here */
								   NULL,
								   NULL);
  if (connection == NULL) {
    g_printerr("Connection failed! Frontend is not active!\n");
    exit(-1);
  }
  GOutputStream * ostream = g_io_stream_get_output_stream (G_IO_STREAM (connection));
  g_output_stream_write  (ostream,
			  message,
                          strlen(message),
                          NULL,
                          NULL);
  g_io_stream_close(G_IO_STREAM (connection),
		    NULL,
		    NULL);
  g_object_unref(connection);
}
