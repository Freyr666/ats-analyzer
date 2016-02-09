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
  PROP_ID,
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
					 "FIXME Long name", "Generic", "FIXME Description",
					 "FIXME <fixme@example.com>");

  gobject_class->set_property = gst_videoanalysis_set_property;
  gobject_class->get_property = gst_videoanalysis_get_property;
  gobject_class->dispose = gst_videoanalysis_dispose;
  gobject_class->finalize = gst_videoanalysis_finalize;
  base_transform_class->start = GST_DEBUG_FUNCPTR (gst_videoanalysis_start);
  base_transform_class->stop = GST_DEBUG_FUNCPTR (gst_videoanalysis_stop);
  video_filter_class->set_info = GST_DEBUG_FUNCPTR (gst_videoanalysis_set_info);
  video_filter_class->transform_frame_ip = GST_DEBUG_FUNCPTR (gst_videoanalysis_transform_frame_ip);

  properties [PROP_ID] =
    g_param_spec_string("id",
			"Id",
			"Channel id (channel num)",
			NULL,
			(G_PARAM_READWRITE));
  properties [PROP_PERIOD] =
    g_param_spec_string("period",
			"Period",
			"Number of frames, which forces filter to emit the info massege",
			NULL,
			(G_PARAM_READWRITE));
  properties [PROP_BLACK] =
    g_param_spec_string("black_lb",
			"Black_lb",
			"Blacness lower bounder",
			NULL,
			(G_PARAM_READWRITE));
  properties [PROP_FREEZE] =
    g_param_spec_string("freeze_lb",
			"Freeze_lb",
			"Freeze lower bounder",
			NULL,
			(G_PARAM_READWRITE));
  g_object_class_install_properties(gobject_class, LAST_PROP, properties);

}

static void
gst_videoanalysis_init (GstVideoAnalysis *videoanalysis)
{
  videoanalysis->id = 2000;
  videoanalysis->counter = 0;
  videoanalysis->black_lb = 16;
  videoanalysis->freeze_lb = 0;
  videoanalysis->period = 8;
  videoanalysis->past_buffer = NULL;
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
  case PROP_ID:
    videoanalysis->id = g_value_get_uint(value);
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
  case PROP_ID:
    g_value_set_uint(value, videoanalysis->id);
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

  if (videoanalysis->past_buffer != NULL)
    g_free(videoanalysis->past_buffer);

  G_OBJECT_CLASS (gst_videoanalysis_parent_class)->finalize (object);
}

static gboolean
gst_videoanalysis_start (GstBaseTransform * trans)
{
  GstVideoAnalysis *videoanalysis = GST_VIDEOANALYSIS (trans);

  GST_DEBUG_OBJECT (videoanalysis, "start");

  videoanalysis->data = video_data_new(videoanalysis->id, videoanalysis->period);

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
  if (videoanalysis->past_buffer != NULL)
    free(videoanalysis->past_buffer);
  videoanalysis->past_buffer = NULL;
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
    gst_structure_id_set(st,
			 VIDEO_DATA_MARKER,
			 G_TYPE_STRING,
			 video_data_to_string(videoanalysis->data),
			 NULL);
    
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
#define VERSION "0.0.FIXME"
#endif
#ifndef PACKAGE
#define PACKAGE "FIXME_package"
#endif
#ifndef PACKAGE_NAME
#define PACKAGE_NAME "FIXME_package_name"
#endif
#ifndef GST_PACKAGE_ORIGIN
#define GST_PACKAGE_ORIGIN "http://FIXME.org/"
#endif

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    videoanalysis,
    "FIXME plugin description",
    plugin_init, VERSION, "LGPL", PACKAGE_NAME, GST_PACKAGE_ORIGIN)

