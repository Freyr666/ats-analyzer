#ifndef _GST_VIDEOANALYSIS_H_
#define _GST_VIDEOANALYSIS_H_

#include <gst/video/video.h>
#include <gst/video/gstvideofilter.h>

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
  guint id;
  guint period;
  guint black_lb;
  guint freeze_lb;
  /* private */
  guint counter;
  guint8 *past_buffer;

  VideoData *data;
};

struct _GstVideoAnalysisClass
{
  GstVideoFilterClass base_videoanalysis_class;
};

GType gst_videoanalysis_get_type (void);

G_END_DECLS

#endif
