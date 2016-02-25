#include "ats_branch.h"
#include <gst/video/videooverlay.h>
#include <stdlib.h>

typedef struct __callback_data
{
  const ATS_METADATA* data;
  ATS_BRANCH* branch;
  guint xid;
  double volume;
} CALLBACK_DATA;

static ATS_SUBBRANCH*
create_video_bin(const gchar* type,
		 const guint stream,
		 const guint prog,
		 const guint pid,
		 const guint xid)
{
  GstElement *bin, *queue, *parser, *decoder, *analyser, *sink;
  GstPad *pad, *ghost_pad;
  ATS_SUBBRANCH* rval;

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
  g_object_set (G_OBJECT (queue),
		"flush-on-eos", TRUE,
		"max-size-buffers", 20000,
		"max-size-bytes", 12000000,
		NULL);
  // "max-size-bytes", 50000,
  analyser = gst_element_factory_make("videoanalysis", NULL);
  g_object_set(G_OBJECT (analyser),
	       "stream_id", stream,
	       "program", prog,
	       "pid", pid,
	       NULL);
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

  rval = g_new(ATS_SUBBRANCH, 1);
  rval->bin = bin;
  rval->sink = sink;
  rval->analyser = analyser;
  rval->pid = pid;
  rval->type = g_strdup(type);
  return rval;
}

static ATS_SUBBRANCH*
create_audio_bin(const gchar* type,
		 const guint stream,
		 const guint prog,
		 const guint pid,
		 const double volume)
{
  GstElement *bin, *queue, *parser, *decoder, *analyser, *sink;
  GstPad *pad, *ghost_pad;
  ATS_SUBBRANCH* rval;
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
  analyser = NULL;
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

  g_object_set (G_OBJECT (sink),
		"volume", volume,
		NULL);
  
  pad = gst_element_get_static_pad (queue, "sink");
  ghost_pad = gst_ghost_pad_new ("sink", pad);
  gst_pad_set_active (ghost_pad, TRUE);
  gst_element_add_pad (bin, ghost_pad);
  gst_object_unref (GST_OBJECT (pad));

  rval = g_new(ATS_SUBBRANCH, 1);
  rval->bin = bin;
  rval->sink = sink;
  rval->analyser = analyser;
  rval->pid = pid;
  rval->type = g_strdup(type);
  return rval;
}

static void
branch_on_pad_added(GstElement* el,
		    GstPad* pad,
		    gpointer data)
{
  CALLBACK_DATA* cb_data = (CALLBACK_DATA*)data;
  GstCaps *caps;
  GstStructure *pad_struct = NULL;
  GstPad* sinkpad;
  const gchar *pad_type = NULL;
  guint pid_num = 0;
  ATS_SUBBRANCH* tail = NULL;
  gchar** type_tocs;
  gchar** pid_tocs;
  ATS_BRANCH* branch = cb_data->branch;
  const ATS_METADATA* metadata = cb_data->data;
  
  g_print ("Dynamic pad created, linking demuxer/decoder\n");
  g_print ("Received new pad '%s' from '%s':\n", GST_PAD_NAME (pad), GST_ELEMENT_NAME (el));
  /* Getting new pad's caps */
  caps = gst_pad_get_current_caps(pad);
  pad_struct = gst_caps_get_structure (caps, 0);
  pad_type = gst_structure_get_name (pad_struct);
  g_print("Result: %s\n", pad_type);
  /* Getting pad's type and pid */
  pid_tocs = g_strsplit(GST_PAD_NAME (pad), "_", 2);
  pid_num = strtoul(pid_tocs[1], NULL, 16);
  type_tocs = g_strsplit(pad_type, "/", 2);
  g_print("Got %s of type %s\n", type_tocs[0], type_tocs[1]);
  if (ats_metadata_find_pid(metadata, branch->prog_num, pid_num)){
    /* If recieved pad is video pad: */
    if (type_tocs[0][0] == 'v')
      tail = create_video_bin(type_tocs[1],
			      branch->stream_id,
			      branch->prog_num,
			      pid_num,
			      cb_data->xid);
    /* Else if pad is audio pad: */
    else if (type_tocs[0][0] == 'a')
      tail = create_audio_bin(type_tocs[1],
			      branch->stream_id,
			      branch->prog_num,
			      pid_num,
			      cb_data->volume);
    /* Connecting subbranch to the tsdemux element of the branch: */
    if ((tail != NULL) && (tail->bin != NULL)) {
      g_print("Playing pipeline has been created\n");
      branch->subbranches = g_slist_append(branch->subbranches,
					   tail);
      /* Connecting pads: */
      g_print("Playing pipeline has been created\n");
      // gst_bin_sync_children_states(GST_BIN(branch->bin));
      gst_bin_add((GstBin*) branch->bin, tail->bin);
      gst_element_set_state(branch->bin, GST_STATE_PAUSED);
      gst_bin_sync_children_states(GST_BIN(branch->bin));
      gst_element_set_state(branch->bin, GST_STATE_PLAYING);
      sinkpad = gst_element_get_static_pad (tail->bin, "sink");
      gst_pad_link (pad, sinkpad);
      gst_element_set_state(branch->bin, GST_STATE_PLAYING);
      g_print("Linked!\n");
      gst_object_unref(GST_OBJECT(sinkpad));
    }
  }
  g_strfreev(pid_tocs);
  g_strfreev(type_tocs);
}

ATS_BRANCH*
ats_branch_new(const guint stream_id,
	       const guint prog_num,
	       const guint xid,
	       const double volume,
	       const ATS_METADATA* data)
{
  ATS_BRANCH *rval;
  GstPad *pad, *ghost_pad;
  GstElement *queue, *demux;
  CALLBACK_DATA* cb_data;
  
  rval = g_new(ATS_BRANCH, 1);
  rval->stream_id = stream_id;
  rval->prog_num = prog_num;
  rval->bin = gst_bin_new(NULL);
  rval->subbranches = NULL;

  cb_data = g_new(CALLBACK_DATA, 1);
  cb_data->branch = rval;
  cb_data->data = data;
  cb_data->xid = xid;
  cb_data->volume = volume;
  
  queue = gst_element_factory_make("queue2", NULL);
  demux = gst_element_factory_make("tsdemux", NULL);
  
  g_object_set (G_OBJECT (queue),
		"max-size-buffers", 200000,
		"max-size-bytes", 429496729,
		NULL);
  
  g_object_set (G_OBJECT (demux),
		"program-number", rval->prog_num,
		NULL);
  
  gst_bin_add_many(GST_BIN(rval->bin), queue, demux, NULL);
  gst_element_link_many(queue, demux, NULL);
  pad = gst_element_get_static_pad (queue, "sink");
  ghost_pad = gst_ghost_pad_new ("sink", pad);
  gst_pad_set_active (ghost_pad, TRUE);
  gst_element_add_pad (rval->bin, ghost_pad);
  gst_object_unref (pad);

  g_signal_connect(demux, "pad-added", G_CALLBACK (branch_on_pad_added), cb_data);
  return rval;
}

void
ats_branch_delete(ATS_BRANCH* this)
{
  if (this->bin){
    gst_object_unparent(GST_OBJECT(this->bin));
    gst_object_unref(GST_OBJECT(this->bin));
  }
  g_free(this);
}

