#ifndef VIDEOANALYSIS_API_H
#define VIDEOANALYSIS_API_H

#include <glib.h>

#define DATA_MARKER 0x8BA820F0
#define VIDEO_DATA_MARKER 0xEA96C3D8

typedef struct __VideoParams VideoParams;
typedef struct __VideoData VideoData;

struct __VideoParams {
  float frozen_pix;
  float black_pix;
  float blocks;
  float avg_bright;
  float avg_diff;
};

struct __VideoData {
  guint data_marker;
  guint channel;
  guint current;
  guint frames;
  VideoParams* data;
};

VideoData* video_data_new(guint ch, guint fr);
void video_data_reset(VideoData* dt);
void video_data_delete(VideoData* dt);
gint video_data_append(VideoData* dt, VideoParams par);
gboolean video_data_is_full(VideoData* dt);
/* convert data into string 
 * format:
 * channel:*:frozen_pix:black_pix:blocks:avg_bright:avg_diff:*:frozen_pix:black_pix:blocks:avg_bright:avg_diff
 */
gchar* video_data_to_string(VideoData* dt);

#endif /* VIDEOANALYSIS_API_H */
