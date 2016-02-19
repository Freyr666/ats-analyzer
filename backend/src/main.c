#include <gst/gst.h>
#include <gst/mpegts/mpegts.h>
#include <glib.h>

#include "parse_ts.h"
#include "ats_branch.h"
#include "ats_metadata.h"
#include "ats_tree.h"
#include "videoanalysis_api.h"
#include "ats_control.h"

typedef struct __bus_data
{
  GMainLoop* loop;
  ATS_TREE* tree; 
  ATS_CONTROL* control;
} BUS_DATA;

static gboolean
bus_call(GstBus* bus,
	 GstMessage* msg,
	 gpointer data)
{
  BUS_DATA* d = (BUS_DATA*) data;
  GMainLoop* loop = d->loop;
  ATS_TREE* tree = d->tree;
  ATS_CONTROL* control = d->control;
  switch (GST_MESSAGE_TYPE(msg)) {
  case GST_MESSAGE_ERROR: {
    gchar *debug;
    GError *error;
    gst_message_parse_error (msg, &error, &debug);
    g_free (debug);
    g_printerr ("Error: %s\n", error->message);
    g_error_free (error);
    g_main_loop_quit (loop);
    break;
  }
  case GST_MESSAGE_ELEMENT: {
    GstMpegtsSection *section;
    const GstStructure* st;
    if ((section = gst_message_parse_mpegts_section (msg))) {
      if (parse_table (section, tree->metadata) &&
	  ats_metadata_is_ready(tree->metadata) &&
	  tree->branches == NULL){
	gchar* str = ats_metadata_to_string(tree->metadata);
	ats_control_send(control, str);
	g_free(str);
      }
      gst_mpegts_section_unref (section);
    }

    else {
      st = gst_message_get_structure(msg);
      if (gst_structure_has_name (st, "GstUDPSrcTimeout")){
        ats_control_send(control, "e0");
	if (tree->branches != NULL)
	  ats_tree_reset(tree);
      }
      if (gst_structure_get_name_id(st) == DATA_MARKER){
	//gchar* str = g_value_dup_string(gst_structure_id_get_value(st, VIDEO_DATA_MARKER));
	//ats_control_send(control, str);
	//g_free(str);
      }
    }
    break;
  }
  default:
    break;
  }
  return TRUE;
}

int
main(int argc,
     char *argv[])
{
  GMainLoop *mainloop;
  GstBus* bus;
  ATS_TREE* proctree;
  ATS_CONTROL* control;
  BUS_DATA* data;
  
  gst_init(&argc, &argv);
  mainloop = g_main_loop_new(NULL, FALSE);

  proctree = ats_tree_new(0);
  bus = ats_tree_get_bus(proctree);
  ats_tree_set_state(proctree, GST_STATE_PLAYING);
  control = ats_control_new(proctree);
  mainloop = g_main_loop_new (NULL, FALSE);
  
  data = g_new(BUS_DATA, 1);
  data->loop = mainloop;
  data->tree = proctree;
  data->control = control;
  gst_bus_add_watch(bus, bus_call, data);
  
  g_print ("Now playing: %s\nRunning...\n", argv[1]);
  g_main_loop_run (mainloop);

  g_print ("Returned, stopping playback\n");
  ats_tree_set_state(proctree, GST_STATE_NULL);
  g_print ("Deleting pipeline\n");
  g_main_loop_unref (mainloop);
  return 0;
}

