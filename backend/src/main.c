#include <gst/gst.h>
#include <gst/mpegts/mpegts.h>
#include <glib.h>

#include "parse_ts.h"
#include "ats_branch.h"
#include "ats_metadata.h"
#include "ats_tree.h"
#include "videoanalysis_api.h"

typedef struct __bus_data
{
  GMainLoop* loop;
  ATS_TREE* tree;
} BUS_DATA;

static gboolean
bus_call(GstBus* bus,
	 GstMessage* msg,
	 gpointer data)
{
  BUS_DATA* d = (BUS_DATA*) data;
  GMainLoop* loop = d->loop;
  ATS_TREE* tree = d->tree;
  switch (GST_MESSAGE_TYPE(msg)) {
  case GST_MESSAGE_EOS: {
    g_print("End of stream\n");
    break;
  }
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
      if(parse_table (section, tree->metadata) && ats_metadata_is_ready(tree->metadata)){
	if (tree->branches == NULL){
	  ats_tree_add_branches(tree);
	}
	/*else {
	  ats_tree_remove_branches(tree);
	  }*/
      }
      gst_mpegts_section_unref (section);
    }

    else {
      st = gst_message_get_structure(msg);
      if (gst_structure_has_name (st, "GstUDPSrcTimeout")){
	g_print("EOS!!!\n");
	if (tree->branches != NULL){
	  ats_tree_remove_branches(tree);
	  ats_tree_set_state(tree, GST_STATE_PLAYING);
	}
      }
      if (gst_structure_get_name_id(st) == DATA_MARKER)
	g_print("%s\n", g_value_dup_string((gst_structure_id_get_value(st, VIDEO_DATA_MARKER))));
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
  BUS_DATA* data;
  
  gst_init(&argc, &argv);
  mainloop = g_main_loop_new(NULL, FALSE);

  proctree = ats_tree_new(0);
  ats_tree_set_source(proctree, "udp://127.0.0.1:1234", "127.0.0.1", 1234);
  bus = ats_tree_get_bus(proctree);
  ats_tree_set_state(proctree, GST_STATE_PLAYING);
  
  mainloop = g_main_loop_new (NULL, FALSE);
  
  data = g_new(BUS_DATA, 1);
  data->loop = mainloop;
  data->tree = proctree;
  gst_bus_add_watch(bus, bus_call, data);
  
  g_print ("Now playing: %s\nRunning...\n", argv[1]);
  g_main_loop_run (mainloop);

  g_print ("Returned, stopping playback\n");
  ats_tree_set_state(proctree, GST_STATE_NULL);
  g_print ("Deleting pipeline\n");
  g_main_loop_unref (mainloop);
  return 0;
}

