#include "videoanalysis_api.h"

VideoData*
video_data_new(guint ch, guint fr)
{
  if (fr == 0) return NULL;
  
  VideoData* rval;
  rval = g_new(VideoData, 1);
  rval->data_marker = VIDEO_DATA_MARKER;
  rval->channel = ch;
  rval->frames = fr;
  rval->current = 0;
  rval->data = g_new(VideoParams, fr);
  return rval;
}

void
video_data_reset(VideoData* dt)
{
  dt->current = 0;
}
  
void
video_data_delete(VideoData* dt)
{
  g_free(dt->data);
  g_free(dt);
}

gint
video_data_append(VideoData* dt, VideoParams par)
{
  if(dt->current == dt->frames) return -1;
  guint i = dt->current;
  dt->data[i] = par;
  dt->current++;
  return 0;
}

gboolean
video_data_is_full(VideoData* dt)
{
  if (dt->current == dt->frames) return TRUE;
  return FALSE;
}

gchar*
video_data_to_string(VideoData* dt)
{
  guint i;
  gchar* string;

  string = g_strdup_printf("%d", dt->channel);
  for (i = 0; i < dt->frames; i++) {
    gchar* pr_str = string;
    gchar* tmp = g_strdup_printf(":*:%f:%f:%f:%f:%f",
				 dt->data[i].frozen_pix,
				 dt->data[i].black_pix,
				 dt->data[i].blocks,
				 dt->data[i].avg_bright,
				 dt->data[i].avg_diff);
    string = g_strconcat(pr_str, tmp, NULL);
    g_free(tmp);
    g_free(pr_str);
  }
  return string;
}
