/* GStreamer
 * Copyright (C) 2016 FIXME <fixme@example.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Suite 500,
 * Boston, MA 02110-1335, USA.
 */
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
#include "gstaudioanalysis.h"

GST_DEBUG_CATEGORY_STATIC (gst_audioanalysis_debug_category);
#define GST_CAT_DEFAULT gst_audioanalysis_debug_category

/* prototypes */


static void gst_audioanalysis_set_property (GObject * object,
    guint property_id, const GValue * value, GParamSpec * pspec);
static void gst_audioanalysis_get_property (GObject * object,
    guint property_id, GValue * value, GParamSpec * pspec);
static void gst_audioanalysis_dispose (GObject * object);
static void gst_audioanalysis_finalize (GObject * object);

static gboolean gst_audioanalysis_setup (GstAudioFilter * filter,
    const GstAudioInfo * info);
static GstFlowReturn gst_audioanalysis_transform_ip (GstBaseTransform * trans,
    GstBuffer * buf);

enum
{
  PROP_0
};

/* pad templates */

/* FIXME add/remove the formats that you want to support */
static GstStaticPadTemplate gst_audioanalysis_src_template =
GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("audio/x-raw,format=S16LE,rate=[1,max],"
      "channels=[1,max],layout=interleaved")
    );

/* FIXME add/remove the formats that you want to support */
static GstStaticPadTemplate gst_audioanalysis_sink_template =
GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("audio/x-raw,format=S16LE,rate=[1,max],"
      "channels=[1,max],layout=interleaved")
    );


/* class initialization */

G_DEFINE_TYPE_WITH_CODE (GstAudioanalysis, gst_audioanalysis, GST_TYPE_AUDIO_FILTER,
  GST_DEBUG_CATEGORY_INIT (gst_audioanalysis_debug_category, "audioanalysis", 0,
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
      "FIXME Long name", "Generic", "FIXME Description",
      "FIXME <fixme@example.com>");

  gobject_class->set_property = gst_audioanalysis_set_property;
  gobject_class->get_property = gst_audioanalysis_get_property;
  gobject_class->dispose = gst_audioanalysis_dispose;
  gobject_class->finalize = gst_audioanalysis_finalize;
  audio_filter_class->setup = GST_DEBUG_FUNCPTR (gst_audioanalysis_setup);
  base_transform_class->transform_ip = GST_DEBUG_FUNCPTR (gst_audioanalysis_transform_ip);

}

static void
gst_audioanalysis_init (GstAudioanalysis *audioanalysis)
{
}

void
gst_audioanalysis_set_property (GObject * object, guint property_id,
    const GValue * value, GParamSpec * pspec)
{
  GstAudioanalysis *audioanalysis = GST_AUDIOANALYSIS (object);

  GST_DEBUG_OBJECT (audioanalysis, "set_property");

  switch (property_id) {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

void
gst_audioanalysis_get_property (GObject * object, guint property_id,
    GValue * value, GParamSpec * pspec)
{
  GstAudioanalysis *audioanalysis = GST_AUDIOANALYSIS (object);

  GST_DEBUG_OBJECT (audioanalysis, "get_property");

  switch (property_id) {
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

  /* clean up object here */

  G_OBJECT_CLASS (gst_audioanalysis_parent_class)->finalize (object);
}

static gboolean
gst_audioanalysis_setup (GstAudioFilter * filter, const GstAudioInfo * info)
{
  GstAudioanalysis *audioanalysis = GST_AUDIOANALYSIS (filter);

  GST_DEBUG_OBJECT (audioanalysis, "setup");

  return TRUE;
}

/* transform */
static GstFlowReturn
gst_audioanalysis_transform_ip (GstBaseTransform * trans, GstBuffer * buf)
{
  GstAudioanalysis *audioanalysis = GST_AUDIOANALYSIS (trans);

  GST_DEBUG_OBJECT (audioanalysis, "transform_ip");

  return GST_FLOW_OK;
}

static gboolean
plugin_init (GstPlugin * plugin)
{

  /* FIXME Remember to set the rank if it's an element that is meant
     to be autoplugged by decodebin. */
  return gst_element_register (plugin, "audioanalysis", GST_RANK_NONE,
      GST_TYPE_AUDIOANALYSIS);
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
    audioanalysis,
    "FIXME plugin description",
    plugin_init, VERSION, "LGPL", PACKAGE_NAME, GST_PACKAGE_ORIGIN)

