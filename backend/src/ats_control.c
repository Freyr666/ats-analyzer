#include "ats_control.h"

#include <string.h>
#include <stdlib.h>

struct video_options {
  const gchar* option;
  guint value;
};

static void
set_video_option(gpointer data,
		 gpointer user_data)
{
  ATS_BRANCH* branch = (ATS_BRANCH*) data;
  ATS_SUBBRANCH* subbranch;
  struct video_options* opts = (struct video_options*) user_data;
  guint len = g_slist_length(branch->subbranches);
  for (guint i = 0; i < len; i++) {
    subbranch = g_slist_nth_data(branch->subbranches, i);
    if (subbranch->av != 'v') continue;
    g_object_set(subbranch->analyser,
		 opts->option, opts->value,
		 NULL);
  }
}

static gboolean
parse_tree_message(guint *message,
		   ATS_TREE* tree)
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
  ats_tree_add_branches(tree);
  return FALSE;
}

static gboolean
parse_sound_message(guint* message,
		    ATS_TREE* tree)
{
  ATS_SUBBRANCH* branch;
  guint channel = 0;
  guint pid = 0;
  guint volume = 0;
  double fvolume = .0;
  if ((message[0] != SOUND_HEADER) ||
      (message[4] != SOUND_HEADER)) {
    g_print("Not a proper sound message!\n");
    return FALSE;
  }
  channel = message[1];
  pid = message[2];
  volume = message[3];
  if (volume > 100) {
    g_print("Volume value is %d, but it should be in range (0..100)!\n", volume);
    return FALSE;
  }
  fvolume = (double)volume/100.0;
  branch = ats_tree_find_subbranch(tree, channel, pid);
  if(!(branch) ||
     (branch->av != 'a')){
    return FALSE;
  }
  g_object_set(branch->sink,
	       "volume", fvolume,
	       NULL);
  return TRUE;
}

static gboolean
parse_settings_message(guint* message,
		       ATS_TREE* tree)
{
  const static gchar* black = "black_lb";
  const static gchar* freeze = "freeze_lb";
  const static gchar* mark_blocks = "mark_blocks";
  struct video_options opts;
  if ((message[0] != VIDEO_SETTINGS_HEADER) ||
      (message[3] != VIDEO_SETTINGS_HEADER)) {
    g_print("Not a proper settings message!\n");
    return FALSE;
  }
  switch (message[1]) {
  case SETTINGS_BLACK_LEVEL: {
    opts.option = black;
    break;
  }
  case SETTINGS_DIFF_LEVEL: {
    opts.option = freeze;
    break;
  }
  case SETTINGS_MARK_BLOCKS: {
    opts.option = mark_blocks;
    break;
  }
  default: {
    g_print("Wrong option!\n");
    return FALSE;
    break;
  }
  }
  opts.value = message[2];
  if (opts.value > 255) {
    g_print("Value is %d, but it should be in range (0..255)!\n", opts.value);
    return FALSE;
  }
  g_slist_foreach(tree->branches, set_video_option, &opts);
  
  return TRUE;
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
  switch (*message) {
  case TREE_HEADER: {
    return parse_tree_message(message, tree);
    break;
  }
  case SOUND_HEADER:
    return parse_sound_message(message, tree);
    break;
  case VIDEO_SETTINGS_HEADER:
    return parse_settings_message(message, tree);
    break;
  default:
    break;
  }
  g_printerr("Error: unknown message type!\n");
  return FALSE;
}

ATS_CONTROL*
ats_control_init(ATS_CONTROL* ctrl,
		 ATS_TREE* tree,
		 guint stream_id,
		 GError** error)
{
  GError *tmp_error;

  if (ctrl == NULL) {
    g_set_error(error,
		G_ERR_UNKNOWN,
		-1,
		"Error: empty object been passed to ats_control_init");
    return NULL;
  }
  
  tmp_error = NULL;

  ctrl->incoming_service = NULL;
  ctrl->client = NULL;
  /* input */
  ctrl->incoming_service = g_socket_service_new ();
  g_socket_listener_add_inet_port ((GSocketListener*) ctrl->incoming_service,
				   1500+stream_id, /* your port goes here */
				   NULL,
				   &tmp_error);
  if (tmp_error != NULL) {
    goto error;
  }
  
  g_signal_connect (ctrl->incoming_service,
                    "incoming",
                    G_CALLBACK (incoming_callback),
                    tree);
  g_socket_service_start (ctrl->incoming_service);

  return ctrl;
  
 error:
  g_propagate_error(error, tmp_error);
  g_free(ctrl->incoming_service);
  g_free(ctrl->client);
  return NULL;
}

ATS_CONTROL*
ats_control_new(ATS_TREE* tree,
		guint stream_id,
		GError** error)
{
  ATS_CONTROL* rval, *tmp;
  GError* tmp_error;

  tmp_error = NULL;
  rval = g_try_new(ATS_CONTROL, 1);

  tmp = ats_control_init(rval,
			 tree,
			 stream_id,
			 &tmp_error);
  if (tmp == NULL) {
    goto error;
  }

  return rval;

 error:
  g_propagate_error(error, tmp_error);
  g_free(rval);
  return NULL;
}

void
ats_control_delete(ATS_CONTROL* this)
{
  g_free(this);
}

void
ats_control_send(ATS_CONTROL* this,
		 gchar* message,
		 GError** error)
{
  GSocketConnection* connection;
  GOutputStream * ostream;
  GError* tmp_error;

  tmp_error = NULL;
  connection = g_socket_client_connect_to_host (this->client,
						(gchar*)"localhost",
						1600, /* your port goes here */
						NULL,
						NULL);

  if (connection == NULL) {
    /* Exits if it is not possible to send message. */
    g_printerr("Connection failed! Frontend is not active!\n");
    exit(-1);
  }

  ostream = g_io_stream_get_output_stream (G_IO_STREAM (connection));

  g_output_stream_write  (ostream,
			  message,
			  strlen(message),
			  NULL,
			  NULL);
  
  g_io_stream_close(G_IO_STREAM (connection),
		    NULL,
		    &tmp_error);

  if (tmp_error != NULL) {
    g_propagate_error(error, tmp_error);
  }
  
  g_object_unref(connection);
}
