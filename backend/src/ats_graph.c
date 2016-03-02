#include "ats_graph.h"

static gboolean
send_metadata(gpointer data)
{
  ATS_GRAPH* graph = (ATS_GRAPH*) data;
  if (graph->tree->branches == NULL){
    if (ats_metadata_are_ready(graph->tree->metadata)) {
      if(graph->time == 0)
	graph->time = time(0);
      time_t tmp_time = time(0);
      if (((tmp_time - graph->time) >= 2) ||
	  ats_metadata_got_sdt(graph->tree->metadata)){
	gchar* str = ats_metadata_to_string(graph->tree->metadata);
	ats_control_send(graph->control, str);
	graph->metadata_were_sent = TRUE;
	g_free(str);
	return FALSE;
      }
      /* No sdt, time left */
      else
	return TRUE;
    }
    /* Metadata are not ready */
    else
      return TRUE;
  }
  /* Graph is already constructed */
  else
    return FALSE;
}

static gboolean
bus_call(GstBus* bus,
	 GstMessage* msg,
	 gpointer data)
{
  ATS_GRAPH* d = (ATS_GRAPH*) data;
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
    if ((section = gst_message_parse_mpegts_section (msg)) &&
	!(d->metadata_were_sent)) {
      parse_table (section, tree->metadata);
      ats_metadata_print(tree->metadata);
      gst_mpegts_section_unref (section);
    }
    else {
      st = gst_message_get_structure(msg);
      if (gst_structure_has_name (st, "GstUDPSrcTimeout")){
	gchar* str = g_strdup_printf("e%d", tree->metadata->stream_id);
        ats_control_send(control, str);
	if (tree->branches != NULL)
	  ats_tree_remove_branches(tree);
	g_free(str);
      }
      if (gst_structure_get_name_id(st) == DATA_MARKER){
	gchar* str = g_value_dup_string(gst_structure_id_get_value(st, VIDEO_DATA_MARKER));
	ats_control_send(control, str);
	g_free(str);
      }
    }
    break;
  }
  default:
    break;
  }
  return TRUE;
}

ATS_GRAPH*
ats_graph_new(guint stream_id,
	      gchar* ip,
	      guint port)
{
  ATS_GRAPH* rval;
  GstBus* bus;
  rval = g_new(ATS_GRAPH, 1);
  rval->tree = ats_tree_new(stream_id, ip, port);
  rval->loop = g_main_loop_new(NULL, FALSE);
  rval->control = ats_control_new(rval->tree, stream_id);
  rval->time = 0;
  rval->metadata_were_sent = FALSE;
  
  bus = ats_tree_get_bus(rval->tree);

  ats_tree_set_state(rval->tree, GST_STATE_PLAYING);
  
  gst_bus_add_watch(bus, bus_call, rval);
  g_idle_add(send_metadata, rval);
  
  gst_object_unref(bus);
  return rval;
}

void
ats_graph_run(ATS_GRAPH* this)
{
  g_print ("Now playing: \nRunning...\n");
  g_main_loop_run(this->loop);
}

void
ats_graph_delete(ATS_GRAPH* this)
{
  g_print ("Returned, stopping playback\n");
  ats_tree_set_state(this->tree, GST_STATE_NULL);
  g_print ("Deleting pipeline\n");
  g_main_loop_unref (this->loop);

  ats_control_delete(this->control);
  ats_tree_delete(this->tree);
}

