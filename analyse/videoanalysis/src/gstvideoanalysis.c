/* gstvideoanalysis.c
 *
 * Copyright (C) 2016 freyr <sky_rider_93@mail.ru> 
 *
 * This file is free software; you can redistribute it and/or modify it 
 * under the terms of the GNU Lesser General Public License as 
 * published by the Free Software Foundation; either version 3 of the 
 * License, or (at your option) any later version. 
 *
 * This file is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
 * Lesser General Public License for more details. 
 * 
 * You should have received a copy of the GNU General Public License 
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. 
*/

/**
 * SECTION:element-gstvideoanalysis
 *
 * The videoanalysis element does FIXME stuff.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch-1.0 -v fakesrc ! videoanalysis ! FIXME ! fakesink
 * ]|
 * FIXME Describe what the pipeline does.
 * </refsect2>
 */


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gst/gst.h>
#include <gst/video/video.h>
#include <gst/video/gstvideofilter.h>
#include <string.h>
#include <malloc.h>

#include "gstvideoanalysis.h"

#include "analysis.h"

GST_DEBUG_CATEGORY_STATIC (gst_videoanalysis_debug_category);
#define GST_CAT_DEFAULT gst_videoanalysis_debug_category

/* prototypes */

static void
gst_videoanalysis_set_property (GObject * object,
				guint property_id,
				const GValue * value,
				GParamSpec * pspec);
static void
gst_videoanalysis_get_property (GObject * object,
				guint property_id,
				GValue * value,
				GParamSpec * pspec);
static void
gst_videoanalysis_dispose      (GObject * object);
static void
gst_videoanalysis_finalize     (GObject * object);

static gboolean
gst_videoanalysis_start        (GstBaseTransform * trans);
static gboolean
gst_videoanalysis_stop         (GstBaseTransform * trans);
static gboolean
gst_videoanalysis_set_info     (GstVideoFilter * filter,
				GstCaps * incaps,
				GstVideoInfo * in_info,
				GstCaps * outcaps,
				GstVideoInfo * out_info);
static GstFlowReturn
gst_videoanalysis_transform_frame_ip (GstVideoFilter * filter,
				      GstVideoFrame * frame);

enum
{
  PROP_0,
  PROP_STREAM_ID,
  PROP_PROGRAM,
  PROP_PID,
  PROP_PERIOD,
  PROP_BLACK,
  PROP_FREEZE,
  LAST_PROP
};

static GParamSpec *properties[LAST_PROP] = { NULL, };

/* pad templates */

#define VIDEO_SRC_CAPS \
    GST_VIDEO_CAPS_MAKE("{ I420, NV12, NV21, YV12, IYUV }")

#define VIDEO_SINK_CAPS \
    GST_VIDEO_CAPS_MAKE("{ I420, NV12, NV21, YV12, IYUV }")

/* class initialization */

G_DEFINE_TYPE_WITH_CODE (GstVideoAnalysis,
			 gst_videoanalysis,
			 GST_TYPE_VIDEO_FILTER,
			 GST_DEBUG_CATEGORY_INIT (gst_videoanalysis_debug_category,
						  "videoanalysis", 0,
						  "debug category for videoanalysis element"));

static void
gst_videoanalysis_class_init (GstVideoAnalysisClass * klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GstBaseTransformClass *base_transform_class = GST_BASE_TRANSFORM_CLASS (klass);
  GstVideoFilterClass *video_filter_class = GST_VIDEO_FILTER_CLASS (klass);

  /* Setting up pads and setting metadata should be moved to
     base_class_init if you intend to subclass this class. */
  gst_element_class_add_pad_template (GST_ELEMENT_CLASS(klass),
				      gst_pad_template_new ("src",
							    GST_PAD_SRC,
							    GST_PAD_ALWAYS,
							    gst_caps_from_string (VIDEO_SRC_CAPS)));
  gst_element_class_add_pad_template (GST_ELEMENT_CLASS(klass),
				      gst_pad_template_new ("sink",
							    GST_PAD_SINK,
							    GST_PAD_ALWAYS,
							    gst_caps_from_string (VIDEO_SINK_CAPS)));

  gst_element_class_set_static_metadata (GST_ELEMENT_CLASS(klass),
					 "Gstreamer element for video analysis",
					 "Video data analysis", "filter for video analysis",
					 "freyr <sky_rider_93@mail.ru>");

  gobject_class->set_property = gst_videoanalysis_set_property;
  gobject_class->get_property = gst_videoanalysis_get_property;
  gobject_class->dispose = gst_videoanalysis_dispose;
  gobject_class->finalize = gst_videoanalysis_finalize;
  base_transform_class->start = GST_DEBUG_FUNCPTR (gst_videoanalysis_start);
  base_transform_class->stop = GST_DEBUG_FUNCPTR (gst_videoanalysis_stop);
  video_filter_class->set_info = GST_DEBUG_FUNCPTR (gst_videoanalysis_set_info);
  video_filter_class->transform_frame_ip = GST_DEBUG_FUNCPTR (gst_videoanalysis_transform_frame_ip);

  properties [PROP_STREAM_ID] =
    g_param_spec_uint("stream_id",
		      "Stream id",
		      "Plp stream id (stream num)",
		      0,
		      G_MAXUINT,
		      0,
		      G_PARAM_READWRITE);
  properties [PROP_PROGRAM] =
    g_param_spec_uint("program",
		      "Program",
		      "Channel id (channel num)",
		      0,
		      G_MAXUINT,
		      2000,
		      G_PARAM_READWRITE);
  properties [PROP_PID] =
    g_param_spec_uint("pid",
		      "Pid",
		      "Pid id (pid num)",
		      0,
		      G_MAXUINT,
		      2001,
		      G_PARAM_READWRITE);
  properties [PROP_PERIOD] =
    g_param_spec_uint("period",
		      "Period",
		      "Number of frames, which forces filter to emit the info massege",
		      1,
		      1024,
		      8,
		      G_PARAM_READWRITE);
  properties [PROP_BLACK] =
    g_param_spec_uint("black_lb",
		      "Black_lb",
		      "Frame blackness lower bounder",
		      0,
		      256,
		      16,
		      G_PARAM_READWRITE);
  properties [PROP_FREEZE] =
    g_param_spec_uint("freeze_lb",
		      "Freeze_lb",
		      "Frame freeze lower bounder",
		      0,
		      256,
		      0,
		      G_PARAM_READWRITE);
  g_object_class_install_properties(gobject_class, LAST_PROP, properties);

}

static void
gst_videoanalysis_init (GstVideoAnalysis *videoanalysis)
{
  videoanalysis->stream_id = 0;
  videoanalysis->program = 2000;
  videoanalysis->pid = 2001;
  videoanalysis->counter = 0;
  videoanalysis->black_lb = 16;
  videoanalysis->freeze_lb = 0;
  videoanalysis->period = 8;
  videoanalysis->past_buffer = (guint8*)malloc(4096*4096);
}

void
gst_videoanalysis_set_property (GObject * object,
				guint property_id,
				const GValue * value,
				GParamSpec * pspec)
{
  GstVideoAnalysis *videoanalysis = GST_VIDEOANALYSIS (object);

  GST_DEBUG_OBJECT (videoanalysis, "set_property");

  switch (property_id) {
  case PROP_STREAM_ID:
    videoanalysis->stream_id = g_value_get_uint(value);
    break;
  case PROP_PROGRAM:
    videoanalysis->program = g_value_get_uint(value);
    break;
  case PROP_PID:
    videoanalysis->pid = g_value_get_uint(value);
    break;
  case PROP_BLACK:
    videoanalysis->black_lb = g_value_get_uint(value);
    break;
  case PROP_FREEZE:
    videoanalysis->freeze_lb = g_value_get_uint(value);
    break;
  case PROP_PERIOD:
    videoanalysis->period = g_value_get_uint(value);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    break;
  }
}

void
gst_videoanalysis_get_property (GObject * object,
				guint property_id,
				GValue * value,
				GParamSpec * pspec)
{
  GstVideoAnalysis *videoanalysis = GST_VIDEOANALYSIS (object);

  GST_DEBUG_OBJECT (videoanalysis, "get_property");

  switch (property_id) {
  case PROP_STREAM_ID:
    g_value_set_uint(value, videoanalysis->stream_id);
    break;
  case PROP_PROGRAM:
    g_value_set_uint(value, videoanalysis->program);
    break;
  case PROP_PID:
    g_value_set_uint(value, videoanalysis->pid);
    break; 
  case PROP_BLACK:
    g_value_set_uint(value, videoanalysis->black_lb);
    break;
  case PROP_FREEZE:
    g_value_set_uint(value, videoanalysis->freeze_lb);
    break;
  case PROP_PERIOD: 
    g_value_set_uint(value, videoanalysis->period);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    break;
  }
}

void
gst_videoanalysis_dispose (GObject * object)
{
  GstVideoAnalysis *videoanalysis = GST_VIDEOANALYSIS (object);

  GST_DEBUG_OBJECT (videoanalysis, "dispose");

  G_OBJECT_CLASS (gst_videoanalysis_parent_class)->dispose (object);
}

void
gst_videoanalysis_finalize (GObject * object)
{
  GstVideoAnalysis *videoanalysis = GST_VIDEOANALYSIS (object);

  GST_DEBUG_OBJECT (videoanalysis, "finalize");

  free(videoanalysis->past_buffer);

  G_OBJECT_CLASS (gst_videoanalysis_parent_class)->finalize (object);
}

static gboolean
gst_videoanalysis_start (GstBaseTransform * trans)
{
  GstVideoAnalysis *videoanalysis = GST_VIDEOANALYSIS (trans);

  GST_DEBUG_OBJECT (videoanalysis, "start");

  videoanalysis->data = video_data_new(videoanalysis->period);

  return TRUE;
}

static gboolean
gst_videoanalysis_stop (GstBaseTransform * trans)
{
  GstVideoAnalysis *videoanalysis = GST_VIDEOANALYSIS (trans);

  GST_DEBUG_OBJECT (videoanalysis, "stop");

  if(videoanalysis->data != NULL){
    video_data_delete(videoanalysis->data);
    videoanalysis->data = NULL;
  }
  return TRUE;
}

static gboolean
gst_videoanalysis_set_info (GstVideoFilter * filter,
			    GstCaps * incaps,
			    GstVideoInfo * in_info,
			    GstCaps * outcaps,
			    GstVideoInfo * out_info)
{
  GstVideoAnalysis *videoanalysis = GST_VIDEOANALYSIS (filter);

  GST_DEBUG_OBJECT (videoanalysis, "set_info");

  return TRUE;
}

/* transform */
static GstFlowReturn
gst_videoanalysis_transform_frame_ip (GstVideoFilter * filter,
				      GstVideoFrame * frame)
{
  GstVideoAnalysis *videoanalysis = GST_VIDEOANALYSIS (filter);
  VideoParams params;
  
  GST_DEBUG_OBJECT (videoanalysis, "transform_frame_ip");
  
  params = analyse_buffer(frame->data[0],
			  videoanalysis->past_buffer,
			  frame->info.stride[0],
			  frame->info.width,
			  frame->info.height,
			  videoanalysis->black_lb,
			  videoanalysis->freeze_lb);
  
  if (video_data_is_full(videoanalysis->data)){
    
    GstStructure* st = gst_structure_new_id_empty(DATA_MARKER);
    gchar* str = video_data_to_string(videoanalysis->data,
				      videoanalysis->stream_id,
				      videoanalysis->program,
				      videoanalysis->pid);
    gst_structure_id_set(st,
			 VIDEO_DATA_MARKER,
			 G_TYPE_STRING,
			 str,
			 NULL);

    g_free(str);
    gst_element_post_message(GST_ELEMENT_CAST(filter),
			     gst_message_new_element(GST_OBJECT_CAST(filter),
						     st));
    
    
    video_data_reset(videoanalysis->data); 
  }
  
  video_data_append(videoanalysis->data, params);
  
  if(videoanalysis->past_buffer == NULL)
    videoanalysis->past_buffer = (guint8*)malloc(frame->info.stride[0] * frame->info.height + 1);
  
  return GST_FLOW_OK;
}

static gboolean
plugin_init (GstPlugin * plugin)
{

  /* FIXME Remember to set the rank if it's an element that is meant
     to be autoplugged by decodebin. */
  return gst_element_register (plugin,
			       "videoanalysis",
			       GST_RANK_NONE,
			       GST_TYPE_VIDEOANALYSIS);
}

/* FIXME: these are normally defined by the GStreamer build system.
   If you are creating an element to be included in gst-plugins-*,
   remove these, as they're always defined.  Otherwise, edit as
   appropriate for your external plugin package. */
#ifndef VERSION
#define VERSION "0.1.9"
#endif
#ifndef PACKAGE
#define PACKAGE "videoanalysis"
#endif
#ifndef PACKAGE_NAME
#define PACKAGE_NAME "videoanalysis_package"
#endif
#ifndef GST_PACKAGE_ORIGIN
#define GST_PACKAGE_ORIGIN "https://github.com/Freyr666/ats-analyzer/"
#endif

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    videoanalysis,
    "Package for video data analysing",
    plugin_init, VERSION, "LGPL", PACKAGE_NAME, GST_PACKAGE_ORIGIN)

