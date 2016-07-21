#include "ats_graph.h"
#include "parse_ts.h"

#include <stdlib.h>

#define DIFF_PTS 100

static gboolean
send_metadata(gpointer data)
{
  ATS_GRAPH*   graph;
  time_t       tmp_time;
  gchar*       str;
  
  graph = data;
  
  if (graph->tree->branches == NULL){
    if (ats_metadata_are_ready(graph->tree->metadata)) {

      if(graph->time == 0) graph->time = time(0);

      tmp_time = time(0);

      if (((tmp_time - graph->time) >= SDT_TIMEOUT) ||
	  ats_metadata_got_sdt(graph->tree->metadata)){

	str = ats_metadata_to_string(graph->tree->metadata);
	ats_control_send(graph->control, str, NULL);
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
bus_call(GstBus*     bus,
	 GstMessage* msg,
	 gpointer    data)
{
  gchar*              debug;
  gchar*              str;
  guint               pid_pts;
  guint64             pts_pts;
  glong               diff_time;
  gboolean            parsed;
  GError*             error;
  GstMpegtsSection*   section;
  const GstStructure* st;
  GstStructure*       info;
  GstEvent*           event;
  ATS_GRAPH*          graph;
  GMainLoop*          loop;
  ATS_TREE*           tree;
  ATS_CONTROL*        control;
  SIT                 sit;
  ATS_PID_DATA*       pid_data;
  
  graph    = data;
  loop     = graph->loop;
  tree     = graph->tree;
  control  = graph->control;
  parsed   = FALSE;
  
  switch (GST_MESSAGE_TYPE(msg)) {
  case GST_MESSAGE_ERROR: {
    gst_message_parse_error (msg, &error, &debug);
    g_free (debug);
    g_printerr ("Error: %s\n", error->message);
    g_error_free (error);
    g_main_loop_quit (loop);
    break;
  }
  case GST_MESSAGE_ELEMENT: {
    /* 
     *  Gst Message: MPEG-TS section from tsparse:
     *  (compare first letter of the src element name to
     *   ensure that msg was sended by parse element)
     */
    if ((GST_MESSAGE_SRC_NAME(msg)[0] == 'p') &&
	(section = gst_message_parse_mpegts_section (msg))) {

      /* If sit -- parse sit*/
      if (tree->branches != NULL) {
	parsed = parse_scte (section, &sit);

	if (parsed) {

	  pid_data = ats_metadata_find_pid_no_ch(tree->metadata, sit.pmt_pid);

	  g_print("Got SCTE!\nPid: %d\n", sit.pmt_pid);
	  if (pid_data &&
	      (pid_data->type == PID_TYPE_AUDIO)) {
	    pid_data->ad_pts_time = sit.splice_time;
	    pid_data->ad_active = TRUE;
	    g_print("Setting pid!\n");
	  }
	}
      }
      /* Try to parse pmt, pat, sdt etc */
      if (!(graph->metadata_were_sent)) 
	parsed = parse_table (section, tree->metadata);
      
      gst_mpegts_section_unref (section);
      
    }
    /* Gst Message: another messages */
    else {
      st = gst_message_get_structure(msg);
      /* End Of Stream from udpsrc: */
      if (gst_structure_has_name (st, "GstUDPSrcTimeout")){
	str = g_strdup_printf("e%d", tree->metadata->stream_id);
        ats_control_send(control, str, NULL);
	if (tree->branches != NULL)
	  ats_tree_remove_branches(tree);
	g_free(str);
      } 
      /* Data message from audio/videoanalysis */
      else if (gst_structure_get_name_id(st) == DATA_MARKER){
	str = g_value_dup_string(gst_structure_id_get_value(st, DATA_MARKER));
	ats_control_send(control, str, NULL);
	g_free(str);
      } 
      /* PTS packages from ts demux: */
      else if (gst_structure_has_name(st, "tsdemux") &&
	       gst_structure_has_field(st, "pts")) {

	pid_pts = g_value_get_uint(gst_structure_get_value(st, "pid"));
	pts_pts = g_value_get_uint64(gst_structure_get_value(st, "pts"));

	if ((pid_data = ats_metadata_find_pid_no_ch(tree->metadata, pid_pts)) != NULL &&
	    pid_data->ad_active) {

	  diff_time = labs(pid_data->ad_pts_time - pts_pts);
	  if (pid_data->ad_pts_time >= (glong)pts_pts) {

	    info  = gst_structure_new("ad", "pid", G_TYPE_UINT, pid_pts, NULL);
	    event = gst_event_new_custom(GST_EVENT_CUSTOM_DOWNSTREAM, info);

	    gst_pad_push_event(graph->tree->faketee.pad, event);
	    
	  }
	}
	/* g_print("PTS = %lu on PID = %u\n", pts_pts, pid_pts); */
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
ats_graph_init(ATS_GRAPH* graph,
	       guint stream_id,
	       gchar* ip,
	       guint port,
	       GError** error)
{
  GstBus* bus;

  if (graph == NULL) {
    g_set_error(error,
		G_ERR_UNKNOWN,
		-1,
		"Error: empty object been passed to ats_graph_init");
    return NULL;
  }

  graph->tree = ats_tree_new(stream_id, ip, port, NULL);
  graph->loop = g_main_loop_new(NULL, FALSE);
  graph->control = ats_control_new(graph->tree,
				   stream_id, NULL);
  graph->time = 0;
  graph->metadata_were_sent = FALSE;
  /* graph->sdt_was_sent = FALSE; */

  gst_mpegts_initialize();

  bus = ats_tree_get_bus(graph->tree);

  ats_tree_set_state(graph->tree, GST_STATE_PLAYING);

  gst_bus_add_watch(bus, bus_call, graph);

  g_timeout_add_full (G_PRIORITY_LOW,
		      1000, /* interval, ms */
		      send_metadata,
		      graph,
		      NULL);

  gst_object_unref(bus);

  return graph;
}

ATS_GRAPH*
ats_graph_new(guint stream_id,
	      gchar* ip,
	      guint port,
	      GError** error)
{
  ATS_GRAPH* rval;
  GError*    local_error;

  local_error = NULL;
  rval = g_try_new(ATS_GRAPH, 1);
  
  if (rval == NULL) {
    g_set_error(error,
		G_ERR_UNKNOWN,
		-1,
		"Error: Failed to allocate memory for ATS_GRAPH object");
    return NULL;
  }

  rval = ats_graph_init(rval,
			stream_id,
			ip, port,
			&local_error);
  /* TODO:  */
  return rval;
}

void
ats_graph_run(ATS_GRAPH* this)
{
  g_print ("ATS_GRAPH is running\n");
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

