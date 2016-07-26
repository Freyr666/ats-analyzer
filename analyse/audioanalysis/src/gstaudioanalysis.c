/* gstaudioanalysis.c
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

#ifndef LGPL_LIC
#define LIC "Proprietary"
#define URL "http://www.niitv.ru/"
#else
#define LIC "LGPL"
#define URL "https://github.com/Freyr666/ats-analyzer/"
#endif

/**
 * SECTION:element-gstaudioanalysis
 *
 * The audioanalysis element does FIXME stuff.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch-1.0 -v fakesrc ! audioanalysis ! FIXME ! fakesink
 * ]|
 * FIXME Describe what the pipeline does.
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gst/gst.h>
#include <gst/audio/gstaudiofilter.h>
#include "ebur128.h"
#include "gstaudioanalysis.h"

#define DIFF(x,y)((x > y)?(x-y):(y-x))

GST_DEBUG_CATEGORY_STATIC (gst_audioanalysis_debug_category);
#define GST_CAT_DEFAULT gst_audioanalysis_debug_category

/* prototypes */


static void
gst_audioanalysis_set_property (GObject * object,
				guint property_id,
				const GValue * value,
				GParamSpec * pspec);
static void
gst_audioanalysis_get_property (GObject * object,
				guint property_id,
				GValue * value,
				GParamSpec * pspec);
static void
gst_audioanalysis_dispose (GObject * object);

static void
gst_audioanalysis_finalize (GObject * object);

static gboolean
gst_audioanalysis_setup (GstAudioFilter * filter,
			 const GstAudioInfo * info);
static GstFlowReturn
gst_audioanalysis_transform_ip (GstBaseTransform * trans,
				GstBuffer * buf);
static inline void
gst_audioanalysis_send_string (const gchar* data,
			       GstAudioanalysis* filter);
static inline void
gst_audioanalysis_eval_global (GstBaseTransform * trans,
			       guint ad_flag);
static gboolean
gst_filter_sink_ad_event (GstBaseTransform * parent,
			  GstEvent * event);

enum
{
  PROP_0,
  PROP_STREAM_ID,
  PROP_PROGRAM,
  PROP_PID,
  PROP_AD_TIMEOUT,
  LAST_PROP
};

static GParamSpec *properties[LAST_PROP] = { NULL, };

/* pad templates */

static GstStaticPadTemplate gst_audioanalysis_src_template =
  GST_STATIC_PAD_TEMPLATE ("src",
			   GST_PAD_SRC,
			   GST_PAD_ALWAYS,
			   GST_STATIC_CAPS ("audio/x-raw,format=S16LE,rate=[1,max],"
					    "channels=[1,max],layout=interleaved"));

static GstStaticPadTemplate gst_audioanalysis_sink_template =
  GST_STATIC_PAD_TEMPLATE ("sink",
			   GST_PAD_SINK,
			   GST_PAD_ALWAYS,
			   GST_STATIC_CAPS ("audio/x-raw,format=S16LE,rate=[1,max],"
					    "channels=[1,max],layout=interleaved"));


/* class initialization */

G_DEFINE_TYPE_WITH_CODE (GstAudioanalysis,
			 gst_audioanalysis,
			 GST_TYPE_AUDIO_FILTER,
			 GST_DEBUG_CATEGORY_INIT (gst_audioanalysis_debug_category,
						  "audioanalysis", 0,
						  "debug category for audioanalysis element"));

static void
gst_audioanalysis_class_init (GstAudioanalysisClass * klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GstBaseTransformClass *base_transform_class = GST_BASE_TRANSFORM_CLASS (klass);
  GstAudioFilterClass *audio_filter_class = GST_AUDIO_FILTER_CLASS (klass);

  /* Setting up pads and setting metadata should be moved to
     base_class_init if you intend to subclass this class. */
  gst_element_class_add_pad_template (GST_ELEMENT_CLASS(klass),
				      gst_static_pad_template_get (&gst_audioanalysis_src_template));
  gst_element_class_add_pad_template (GST_ELEMENT_CLASS(klass),
				      gst_static_pad_template_get (&gst_audioanalysis_sink_template));

  gst_element_class_set_static_metadata (GST_ELEMENT_CLASS(klass),
					 "Gstreamer element for audio analysis",
					 "Audio data analysis",
					 "filter for audio analysis",
					 "freyr <sky_rider_93@mail.ru>");

  gobject_class->set_property = gst_audioanalysis_set_property;
  gobject_class->get_property = gst_audioanalysis_get_property;
  gobject_class->dispose = gst_audioanalysis_dispose;
  gobject_class->finalize = gst_audioanalysis_finalize;
  audio_filter_class->setup = GST_DEBUG_FUNCPTR (gst_audioanalysis_setup);
  base_transform_class->transform_ip = GST_DEBUG_FUNCPTR (gst_audioanalysis_transform_ip);
  base_transform_class->sink_event = GST_DEBUG_FUNCPTR (gst_filter_sink_ad_event);
  
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
   properties [PROP_AD_TIMEOUT] =
    g_param_spec_int64("ad_timeout",
		       "AD timeout",
		       "Max duration of the ad interval (seconds)",
		       0,
		       G_MAXINT64,
		       4*60*60,
		       G_PARAM_READWRITE);
  
  g_object_class_install_properties(gobject_class, LAST_PROP, properties);
}

static void
gst_audioanalysis_init (GstAudioanalysis *audioanalysis)
{
  audioanalysis->stream_id = 0;
  audioanalysis->program = 2000;
  audioanalysis->pid = 2001;
  audioanalysis->ad_timeout = 4*60*60;
  audioanalysis->state = NULL;
  audioanalysis->data = NULL;
  audioanalysis->time = 0;
  audioanalysis->glob_state = NULL;
  audioanalysis->glob_ad_flag = FALSE;
  audioanalysis->glob_start = 0;
}

void
gst_audioanalysis_set_property (GObject * object,
				guint property_id,
				const GValue * value,
				GParamSpec * pspec)
{
  GstAudioanalysis *audioanalysis = GST_AUDIOANALYSIS (object);

  GST_DEBUG_OBJECT (audioanalysis, "set_property");

  switch (property_id) {
  case PROP_STREAM_ID:
    audioanalysis->stream_id = g_value_get_uint(value);
    break;
  case PROP_PROGRAM:
    audioanalysis->program = g_value_get_uint(value);
    break;
  case PROP_PID:
    audioanalysis->pid = g_value_get_uint(value);
    break;
  case PROP_AD_TIMEOUT:
    audioanalysis->ad_timeout = g_value_get_int64(value);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    break;
  }
}

void
gst_audioanalysis_get_property (GObject * object,
				guint property_id,
				GValue * value,
				GParamSpec * pspec)
{
  GstAudioanalysis *audioanalysis = GST_AUDIOANALYSIS (object);

  GST_DEBUG_OBJECT (audioanalysis, "get_property");

  switch (property_id) {
  case PROP_STREAM_ID:
    g_value_set_uint(value, audioanalysis->stream_id);
    break;
  case PROP_PROGRAM:
    g_value_set_uint(value, audioanalysis->program);
    break;
  case PROP_PID:
    g_value_set_uint(value, audioanalysis->pid);
    break;
  case PROP_AD_TIMEOUT:
    g_value_set_int64(value, audioanalysis->ad_timeout);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    break;
  }
}

void
gst_audioanalysis_dispose (GObject * object)
{
  GstAudioanalysis *audioanalysis = GST_AUDIOANALYSIS (object);
  
  GST_DEBUG_OBJECT (audioanalysis, "dispose");

  /* clean up as possible.  may be called multiple times */

  G_OBJECT_CLASS (gst_audioanalysis_parent_class)->dispose (object);
}

void
gst_audioanalysis_finalize (GObject * object)
{
  GstAudioanalysis *audioanalysis = GST_AUDIOANALYSIS (object);

  GST_DEBUG_OBJECT (audioanalysis, "finalize");

  if (audioanalysis->state != NULL)
    ebur128_destroy(&audioanalysis->state);
  if (audioanalysis->glob_state != NULL)
    ebur128_destroy(&audioanalysis->glob_state);

  if (audioanalysis->data != NULL)
    audio_data_delete(audioanalysis->data);

  //gst_object_unref(audioanalysis->clock);

  G_OBJECT_CLASS (gst_audioanalysis_parent_class)->finalize (object);
}

static gboolean
gst_audioanalysis_setup (GstAudioFilter * filter,
			 const GstAudioInfo * info)
{
  GstAudioanalysis *audioanalysis = GST_AUDIOANALYSIS (filter);

  GST_DEBUG_OBJECT (audioanalysis, "setup");
  
  if (audioanalysis->state != NULL)
    ebur128_destroy(&audioanalysis->state);
  if (audioanalysis->glob_state != NULL)
    ebur128_destroy(&audioanalysis->glob_state);
  
  audioanalysis->state = ebur128_init(info->channels,
				      (unsigned long)info->rate,
				      EBUR128_MODE_S | EBUR128_MODE_M);
  audioanalysis->glob_state = ebur128_init(info->channels,
					   (unsigned long)info->rate,
					   EBUR128_MODE_I);

  if (audioanalysis->data != NULL)
    audio_data_delete(audioanalysis->data);
  audioanalysis->data = audio_data_new(EVAL_PERIOD);

  audioanalysis->time = gst_clock_get_time(GST_ELEMENT(audioanalysis)->clock);
  
  return TRUE;
}

static inline void
gst_audioanalysis_eval_global (GstBaseTransform * trans,
			       guint ad_flag)
{
  GstAudioanalysis *audioanalysis;
  double           result, diff_time;
  time_t           now;
  gchar            string[50];

  audioanalysis = GST_AUDIOANALYSIS (trans);
  now = time(NULL);

  /* if measurements have already begun */
  if (audioanalysis->glob_ad_flag) {

    ebur128_loudness_global(audioanalysis->glob_state, &result);
    ebur128_clear_block_list(audioanalysis->glob_state);

    diff_time = difftime(audioanalysis->glob_start, now);

    g_snprintf(string, 39, "c%d:%d:%d:*:%.2f:%.2f:%d",
	       audioanalysis->stream_id,
	       audioanalysis->program,
	       audioanalysis->pid,
	       result, diff_time, ad_flag);

    gst_audioanalysis_send_string(string, audioanalysis);
  } else {
    audioanalysis->glob_ad_flag = TRUE;
  }

  audioanalysis->glob_start = now;
}

/* send data */
static inline void
gst_audioanalysis_send_string(const gchar* data,
			      GstAudioanalysis* filter)
{
  if (data != NULL){
    GstStructure* st = gst_structure_new_id_empty(DATA_MARKER);
    
    gst_structure_id_set(st,
			 DATA_MARKER,
			 G_TYPE_STRING,
			 data,
			 NULL);
    
    gst_element_post_message(GST_ELEMENT_CAST(filter),
			     gst_message_new_element(GST_OBJECT_CAST(filter),
						     st));
  }
  return;
}

/* transform */
static GstFlowReturn
gst_audioanalysis_transform_ip (GstBaseTransform * trans,
				GstBuffer * buf)
{
  GstAudioanalysis *audioanalysis = GST_AUDIOANALYSIS (trans);
  GstMapInfo map;
  guint num_frames;
  time_t now;
  AudioParams params;
  gchar* rval = NULL;
  GstClockTime current_time = gst_clock_get_time(GST_ELEMENT(audioanalysis)->clock);
  
  GST_DEBUG_OBJECT (audioanalysis, "transform_ip");
  
  gst_buffer_map(buf, &map, GST_MAP_READ);
  num_frames = map.size / (GST_AUDIO_FILTER_BPS(audioanalysis) * GST_AUDIO_FILTER_CHANNELS(audioanalysis));

  ebur128_add_frames_short(audioanalysis->state, (short*)map.data, num_frames);

  /* add frames to an ad state */
  if (audioanalysis->glob_ad_flag) {

    now = time(NULL);
    
    ebur128_add_frames_short(audioanalysis->glob_state, (short*)map.data, num_frames);

    /* interval exceeded specified timeout */
    if (DIFF(now, audioanalysis->glob_start) >= audioanalysis->ad_timeout) {
      ebur128_clear_block_list(audioanalysis->glob_state);
      audioanalysis->glob_ad_flag = FALSE;
    }
  }
    
  /* send data for the momentary and short term states */
  if (audio_data_is_full(audioanalysis->data)) {
    rval = audio_data_to_string(audioanalysis->data,
				audioanalysis->stream_id,
				audioanalysis->program,
				audioanalysis->pid);
    gst_audioanalysis_send_string(rval, audioanalysis);
    audio_data_reset(audioanalysis->data);
    g_free(rval);
  }

  /* eval loudness for the 100ms interval */
  if (DIFF(current_time, audioanalysis->time) >= OBSERVATION_TIME) {
    
    ebur128_loudness_momentary(audioanalysis->state, &(params.moment));
    ebur128_loudness_shortterm(audioanalysis->state, &(params.shortt));

    audio_data_append(audioanalysis->data, &params);
    audioanalysis->time = current_time;
  }
  
  gst_buffer_unmap(buf, &map);

  return GST_FLOW_OK;
}

static gboolean
gst_filter_sink_ad_event (GstBaseTransform * base,
			  GstEvent * event)
{
  GstAudioanalysis       *filter;
  const GstStructure     *st; 
  guint                  pid;
  guint                  ad;
  
  filter = GST_AUDIOANALYSIS(base);
  
  if (GST_EVENT_TYPE (event) == GST_EVENT_CUSTOM_DOWNSTREAM) { 

    st = gst_event_get_structure(event);

    if (gst_structure_has_name(st, "ad")) {
      
      pid = g_value_get_uint(gst_structure_get_value(st, "pid"));
      ad  = g_value_get_uint(gst_structure_get_value(st, "isad"));
      
      if (filter->program == pid) {
	
	gst_audioanalysis_eval_global(base, ad);
	
	gst_event_unref(event);
	event = NULL;
      }
    }
  }
  /* pass event on */
  if (event)
    return GST_BASE_TRANSFORM_CLASS
      (gst_audioanalysis_parent_class)->sink_event (base, event);
  else 
    return TRUE;  
}

static gboolean
plugin_init (GstPlugin * plugin)
{

  /* FIXME Remember to set the rank if it's an element that is meant
     to be autoplugged by decodebin. */
  return gst_element_register (plugin,
			       "audioanalysis",
			       GST_RANK_NONE,
			       GST_TYPE_AUDIOANALYSIS);
}

/* FIXME: these are normally defined by the GStreamer build system.
   If you are creating an element to be included in gst-plugins-*,
   remove these, as they're always defined.  Otherwise, edit as
   appropriate for your external plugin package. */
#ifndef VERSION
#define VERSION "0.1.9"
#endif
#ifndef PACKAGE
#define PACKAGE "audioanalysis"
#endif
#ifndef PACKAGE_NAME
#define PACKAGE_NAME "audioanalysis_package"
#endif
#ifndef GST_PACKAGE_ORIGIN
#define GST_PACKAGE_ORIGIN URL
#endif

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
		   GST_VERSION_MINOR,
		   audioanalysis,
		   "Package for audio data analysis",
		   plugin_init, VERSION, LIC, PACKAGE_NAME, GST_PACKAGE_ORIGIN)

