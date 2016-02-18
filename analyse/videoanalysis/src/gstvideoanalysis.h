/* gstvideoanalysis.h
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

#ifndef _GST_VIDEOANALYSIS_H_
#define _GST_VIDEOANALYSIS_H_

#include <gst/video/video.h>
#include <gst/video/gstvideofilter.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/file.h>

#include "videoanalysis_api.h"

G_BEGIN_DECLS

#define GST_TYPE_VIDEOANALYSIS\
  (gst_videoanalysis_get_type())
#define GST_VIDEOANALYSIS(obj)\
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_VIDEOANALYSIS,GstVideoAnalysis))
#define GST_VIDEOANALYSIS_CLASS(klass)\
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_VIDEOANALYSIS,GstVideoAnalysisClass))
#define GST_IS_VIDEOANALYSIS(obj)\
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_VIDEOANALYSIS))
#define GST_IS_VIDEOANALYSIS_CLASS(obj)\
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_VIDEOANALYSIS))

typedef struct _GstVideoAnalysis GstVideoAnalysis;
typedef struct _GstVideoAnalysisClass GstVideoAnalysisClass;

struct _GstVideoAnalysis
{
  GstVideoFilter base_videoanalysis;
  /* public */
  guint stream_id;
  guint program;
  guint pid;
  guint period;
  guint black_lb;
  guint freeze_lb;
  /* private */
  guint counter;
  guint8 *past_buffer;
  VideoData *data;

  int socket;
  struct sockaddr_in addr;
};

struct _GstVideoAnalysisClass
{
  GstVideoFilterClass base_videoanalysis_class;
};

GType gst_videoanalysis_get_type (void);

G_END_DECLS

#endif
