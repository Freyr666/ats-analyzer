/* gstaudioanalysis.h
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

#ifndef _GST_AUDIOANALYSIS_H_
#define _GST_AUDIOANALYSIS_H_

#include <gst/audio/gstaudiofilter.h>
#include <ebur128.h>

#define DATA_MARKER 0x8BA820F0

G_BEGIN_DECLS

#define GST_TYPE_AUDIOANALYSIS\
  (gst_audioanalysis_get_type())
#define GST_AUDIOANALYSIS(obj)\
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_AUDIOANALYSIS,GstAudioanalysis))
#define GST_AUDIOANALYSIS_CLASS(klass)\
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_AUDIOANALYSIS,GstAudioanalysisClass))
#define GST_IS_AUDIOANALYSIS(obj)\
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_AUDIOANALYSIS))
#define GST_IS_AUDIOANALYSIS_CLASS(obj)\
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_AUDIOANALYSIS))

typedef struct _GstAudioanalysis GstAudioanalysis;
typedef struct _GstAudioanalysisClass GstAudioanalysisClass;

struct _GstAudioanalysis
{
  GstAudioFilter base_audioanalysis;
  /* Public */
  guint stream_id;
  guint program;
  guint pid;
  /*guint period;*/
  /* Private */
  ebur128_state* state_momentary;
  ebur128_state* state_short;
};

struct _GstAudioanalysisClass
{
  GstAudioFilterClass base_audioanalysis_class;
};

GType gst_audioanalysis_get_type (void);

G_END_DECLS

#endif
