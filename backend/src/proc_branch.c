#include "proc_branch.h"
#include <gst/video/videooverlay.h>
#include <stdlib.h>

static GstElement*
create_video_bin(const gchar* type,
		 const guint xid,
		 const guint pid)
{
  GstElement *bin, *queue, *parser, *decoder, *analyser, *sink;
  GstPad *pad, *ghost_pad;
  if (g_strcmp0(type, "x-h264") == 0){
    parser = gst_element_factory_make("h264parse", NULL);
    decoder = gst_element_factory_make("avdec_h264", NULL);
  }
  else if (g_strcmp0(type, "mpeg") == 0){
    parser = gst_element_factory_make("mpegvideoparse", NULL);
    decoder = gst_element_factory_make("avdec_mpeg2video", NULL);
  }
  else{
      g_printerr("Error: wrong pad type: %s\n", type);
      return NULL;
  }
  queue = gst_element_factory_make("queue", NULL);
  g_object_set (G_OBJECT (queue), "max-size-buffers", 20000, NULL);
  g_object_set (G_OBJECT (queue), "max-size-bytes", 5000000, NULL);
  analyser = gst_element_factory_make("videoanalysis", NULL);
  g_object_set(G_OBJECT (analyser), "id", pid, NULL);
  sink = gst_element_factory_make("xvimagesink", NULL);
  /* Overlay */
  if (xid != 0)
    gst_video_overlay_set_window_handle (GST_VIDEO_OVERLAY (sink), xid);
  /* ------- */
  bin = gst_bin_new (NULL);
  if (!bin || !parser || !decoder || !sink || !queue){
    gst_object_unref(bin);
    gst_object_unref(parser);
    gst_object_unref(decoder);
    gst_object_unref(sink);
    gst_object_unref(queue);
    g_printerr ("One element could not be created. Exiting.\n");
    return NULL;
  }
  //if (!analyser)
  //  g_printerr ("Analyzer could not be created. Exiting.\n");
  g_object_set (G_OBJECT (sink), "async", FALSE, NULL);
  gst_bin_add_many(GST_BIN(bin), queue, parser, decoder, analyser, sink, NULL);
  gst_element_link_many(queue, parser, decoder, analyser, sink, NULL);
  pad = gst_element_get_static_pad (queue, "sink");
  ghost_pad = gst_ghost_pad_new ("sink", pad);
  gst_pad_set_active (ghost_pad, TRUE);
  gst_element_add_pad (bin, ghost_pad);
  gst_object_unref (pad);

  return bin;
}

static GstElement*
create_audio_bin(const gchar* type)
{
  GstElement *bin, *queue, *parser, *decoder, *sink;
  GstPad *pad, *ghost_pad;
  if (g_strcmp0(type, "x-ac3") == 0)
    decoder = gst_element_factory_make("avdec_ac3_fixed", NULL);
  else if (g_strcmp0(type, "x-eac3") == 0)
    decoder = gst_element_factory_make("avdec_eac3", NULL);
  else if (g_strcmp0(type, "mpeg") == 0){
    decoder = gst_element_factory_make("mpg123audiodec", NULL);
    parser = gst_element_factory_make("mpegaudioparse", NULL);
  }
  else{
    g_printerr("Error: wrong pad type: %s\n", type);
    return NULL;
  }
  queue = gst_element_factory_make("queue", NULL);
  sink = gst_element_factory_make("pulsesink", NULL);
  bin = gst_bin_new (NULL);
  if (!bin || !decoder || !sink || !queue)
    {
      gst_object_unref(bin);
      gst_object_unref(queue);
      gst_object_unref(decoder);
      gst_object_unref(sink);
      g_printerr ("One element could not be created. Exiting.\n");
      return NULL;
    }
  if (g_strcmp0(type, "mpeg") == 0){
    gst_bin_add_many(GST_BIN(bin), queue, parser, decoder, sink, NULL);
    gst_element_link_many(queue, parser, decoder, sink, NULL);
  }
  else{
    gst_bin_add_many(GST_BIN(bin), queue, decoder, sink, NULL);
    gst_element_link_many(queue, decoder, sink, NULL);
  }
  pad = gst_element_get_static_pad (queue, "sink");
  ghost_pad = gst_ghost_pad_new ("sink", pad);
  gst_pad_set_active (ghost_pad, TRUE);
  gst_element_add_pad (bin, ghost_pad);
  gst_object_unref (GST_OBJECT (pad));

  return bin;
}

static void
branch_on_pad_added(GstElement* el,
		    GstPad* pad,
		    gpointer data)
{
  GstCaps *caps;
  GstStructure *pad_struct = NULL;
  GstPad* sinkpad;
  const gchar *pad_type = NULL;
  guint pid_num = 0;
  GstElement *tail;
  gchar** type_tocs;
  gchar** pid_tocs;
  PROC_BRANCH* branch = (PROC_BRANCH*)data;

  g_print ("Dynamic pad created, linking demuxer/decoder\n");
  g_print ("Received new pad '%s' from '%s':\n", GST_PAD_NAME (pad), GST_ELEMENT_NAME (el));
  
  caps = gst_pad_get_current_caps(pad);
  pad_struct = gst_caps_get_structure (caps, 0);
  pad_type = gst_structure_get_name (pad_struct);
  g_print("Result: %s\n", pad_type);
  pid_tocs = g_strsplit(GST_PAD_NAME (pad), "_", 2);
  pid_num = strtoul(pid_tocs[1], NULL, 16);
  type_tocs = g_strsplit(pad_type, "/", 2);
  g_print("Got %s of type %s\n", type_tocs[0], type_tocs[1]);
  if (type_tocs[0][0] == 'v'){
    g_print ("xid: %d\n", branch->xid);
    tail = create_video_bin(type_tocs[1], branch->xid, pid_num);
  }
  else if (type_tocs[0][0] == 'a')
    tail = create_audio_bin(type_tocs[1]);
  else 
    return;
  if (tail){
    g_print("Playing pipeline has been created\n");
    gst_bin_sync_children_states(GST_BIN(branch->bin));
    gst_bin_add((GstBin*) branch->bin, tail);
    gst_element_set_state(branch->bin, GST_STATE_PLAYING);
    gst_bin_sync_children_states(GST_BIN(branch->bin));
    sinkpad = gst_element_get_static_pad (tail, "sink");
    gst_pad_link (pad, sinkpad);
    g_print("Linked!\n");
    gst_object_unref(GST_OBJECT(sinkpad));
    }
}

PROC_BRANCH*
proc_branch_new(const guint stream_id,
		const guint prog_num,
		const guint xid)
{
  PROC_BRANCH *rval;
  GstPad *pad, *ghost_pad;
  GstElement *queue, *demux;
  rval = g_new(PROC_BRANCH, 1);
  rval->stream_id = stream_id;
  rval->prog_num = prog_num;
  rval->xid = xid;
  rval->bin = gst_bin_new(NULL);
  
  queue = gst_element_factory_make("queue2", NULL);
  demux = gst_element_factory_make("tsdemux", NULL);
  g_object_set (G_OBJECT (queue),
		"max-size-buffers", 2000000,
		"max-size-bytes", 429496729,
		NULL);
  g_object_set (G_OBJECT (demux), "program-number", rval->prog_num, NULL);
  
  gst_bin_add_many(GST_BIN(rval->bin), queue, demux, NULL);
  gst_element_link_many(queue, demux, NULL);
  pad = gst_element_get_static_pad (queue, "sink");
  ghost_pad = gst_ghost_pad_new ("sink", pad);
  gst_pad_set_active (ghost_pad, TRUE);
  gst_element_add_pad (rval->bin, ghost_pad);
  gst_object_unref (pad);

  g_signal_connect(demux, "pad-added", G_CALLBACK (branch_on_pad_added), rval);
  return rval;
}

void
proc_branch_delete(PROC_BRANCH* this)
{
  if (this->bin){
    gst_object_unparent(GST_OBJECT(this->bin));
    gst_object_unref(GST_OBJECT(this->bin));
  }
  g_free(this);
}

