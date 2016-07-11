#include "ats_subbranch.h"
#include <gst/video/videooverlay.h>

ATS_SUBBRANCH*
create_video_subbranch(const gchar* type,
		       const guint stream,
		       const guint prog,
		       const guint pid,
		       const guint xid,
		       GError** error)
{
  GstElement     *bin, *queue, *parser, *decoder, *analyser, *sink;
  GstPad         *pad, *ghost_pad;
  ATS_SUBBRANCH  *rval;

  bin       = NULL;
  queue     = NULL;
  parser    = NULL;
  decoder   = NULL;
  analyser  = NULL;
  sink      = NULL;
  pad       = NULL;
  ghost_pad = NULL;
  rval      = NULL;
  
  if (g_strcmp0(type, "x-h264") == 0) {
    parser = gst_element_factory_make("h264parse", NULL);
    decoder = gst_element_factory_make("avdec_h264", NULL);
  } else if (g_strcmp0(type, "mpeg") == 0) {
    parser = gst_element_factory_make("mpegvideoparse", NULL);
    decoder = gst_element_factory_make("avdec_mpeg2video", NULL);
  } else {
    g_set_error(error,
		G_ERR_UNKNOWN, -1,
		"Error: wrong pad type: %s\n", type);
    return NULL;
  }

  if (parser == NULL || decoder == NULL) {
    goto error;
  }
  
  if ((queue = gst_element_factory_make("queue", NULL)) == NULL) {
    goto error;
  }
  
  g_object_set (G_OBJECT (queue),
		"flush-on-eos", TRUE,
		"max-size-buffers", 20000,
		"max-size-bytes", 12000000,
		NULL);
  
  if ((analyser = gst_element_factory_make("videoanalysis", NULL)) == NULL) {
    goto error;
  }
  
  g_object_set(G_OBJECT (analyser),
	       "stream_id", stream,
	       "program", prog,
	       "pid", pid,
	       NULL);

  if ((sink = gst_element_factory_make("xvimagesink", NULL)) == NULL) {
    goto error;
  }
  
  /* Overlay */
  if (xid != 0) {
    gst_video_overlay_set_window_handle (GST_VIDEO_OVERLAY (sink), xid);
    g_object_set (G_OBJECT (sink), "async", FALSE, NULL);
  } else {
    sink = gst_element_factory_make("fakesink", NULL);
  }
    
    /* ------- */
  if ((bin = gst_bin_new (NULL)) == NULL) {
    goto error;
  }

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
  rval->av = 'v';
  rval->pid = pid;
  rval->type = g_strdup(type);
  return rval;
  
 error:
  if (bin)      gst_object_unref(bin);
  if (parser)   gst_object_unref(parser);
  if (decoder)  gst_object_unref(decoder);
  if (sink)     gst_object_unref(sink);
  if (queue)    gst_object_unref(queue);
  if (rval)     g_free(rval);
  g_set_error(error,
	      G_ERR_UNKNOWN, -1,
	      "One element could not be created.");
  return NULL;
}

ATS_SUBBRANCH*
create_audio_subbranch(const gchar* type,
		       const guint stream,
		       const guint prog,
		       const guint pid,
		       const double volume,
		       GError** error)
{
  GstElement     *bin, *queue, *parser, *decoder, *analyser, *sink;
  GstPad         *pad, *ghost_pad;
  ATS_SUBBRANCH  *rval;

  bin       = NULL;
  queue     = NULL;
  parser    = NULL;
  decoder   = NULL;
  analyser  = NULL;
  sink      = NULL;
  pad       = NULL;
  ghost_pad = NULL;
  rval      = NULL;
  
  if (g_strcmp0(type, "x-ac3") == 0) {
    decoder = gst_element_factory_make("avdec_ac3_fixed", NULL);
  } else if (g_strcmp0(type, "x-eac3") == 0) {
    decoder = gst_element_factory_make("avdec_eac3", NULL);
  } else if (g_strcmp0(type, "mpeg") == 0) {
    decoder = gst_element_factory_make("mpg123audiodec", NULL);
    if ((parser = gst_element_factory_make("mpegaudioparse", NULL)) == NULL) {
      goto error;
    }
  } else {
    g_set_error(error,
		G_ERR_UNKNOWN, -1,
		"Error: wrong pad type: %s\n", type);
    return NULL;
  }

  if (decoder == NULL) {
    goto error;
  }
  
  if ((queue = gst_element_factory_make("queue", NULL)) == NULL) {
    goto error;
  }

  if ((analyser = gst_element_factory_make("audioanalysis", NULL)) == NULL) {
    goto error;
  }
  
  g_object_set(G_OBJECT (analyser),
	       "stream_id", stream,
	       "program", prog,
	       "pid", pid,
	       NULL);
  
  if ((sink = gst_element_factory_make("pulsesink", NULL)) == NULL) {
    goto error;
  }
  
  if ((bin = gst_bin_new (NULL)) == NULL) {
    goto error;
  }
  
  if (g_strcmp0(type, "mpeg") == 0) {
    gst_bin_add_many(GST_BIN(bin), queue, parser, decoder, analyser, sink, NULL);
    gst_element_link_many(queue, parser, decoder, analyser, sink, NULL);
  } else {
    gst_bin_add_many(GST_BIN(bin), queue, decoder, analyser, sink, NULL);
    gst_element_link_many(queue, decoder, analyser, sink, NULL);
  }

  g_object_set (G_OBJECT (sink),
		"volume", volume,
		NULL);
  
  //g_object_set (G_OBJECT (sink), "async", FALSE, NULL);
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
  rval->av = 'a';
  rval->type = g_strdup(type);
  return rval;

 error:
  if (bin)      gst_object_unref(bin);
  if (parser)   gst_object_unref(parser);
  if (decoder)  gst_object_unref(decoder);
  if (sink)     gst_object_unref(sink);
  if (queue)    gst_object_unref(queue);
  if (rval)     g_free(rval);
  g_set_error(error,
	      G_ERR_UNKNOWN, -1,
	      "One element could not be created.");
  return NULL;
}

void
ats_subbranch_delete(ATS_SUBBRANCH* sub)
{
  /* TODO: delete subbranch */
  return;
}
