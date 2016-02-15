#include "ats_metadata.h"

#include <stdio.h>

ATS_METADATA*
ats_metadata_new(guint stream_id)
{
  ATS_METADATA* rval = g_new(ATS_METADATA, 1);
  rval->stream_id = stream_id;
  rval->prog_info = NULL;
  return rval;
}

void
ats_metadata_delete(ATS_METADATA* this)
{
  g_slist_free(this->prog_info);
  g_free(this);
  this = NULL;
}

ATS_CH_DATA*
ats_metadata_find_channel(ATS_METADATA* this, guint num)
{
  ATS_CH_DATA* rval;
  GSList* list;
  if (this == NULL) return NULL;
  list = this->prog_info;
  while (list != NULL){
    rval = list->data;
    if (rval->number == num) return rval;
    list = g_slist_next(list);
  }
  return NULL;
}

ATS_PID_DATA*
ats_metadata_find_pid(ATS_METADATA* data, guint ch, guint pid)
{
  ATS_PID_DATA* rval;
  ATS_CH_DATA* tmpch = ats_metadata_find_channel(data, ch);
  if (tmpch == NULL) return NULL;
  for (guint i = 0; i < tmpch->pids_num; i++) {
    rval = &tmpch->pids[i];
    if (rval->pid == pid) return rval;
  }
  return NULL;
}

gboolean
ats_metadata_is_ready(ATS_METADATA* data)
{
  guint len;
  if (data == NULL) return FALSE;
  if (data->prog_info == NULL) return FALSE;
  len = g_slist_length(data->prog_info);
  for (guint i = 0; i < len; i++) {
    ATS_CH_DATA* ch = g_slist_nth_data(data->prog_info, i);
    if (ch->pids_num == 0) return FALSE;
  }
  return TRUE;
}

gchar*
ats_metadata_to_string(ATS_METADATA* data)
{
  gchar* string;
  guint len;
  string = g_strdup_printf("d%d", data->stream_id);
  len = ats_metadata_ch_number(data);
  for (guint i = 0; i < len; i++) {
    ATS_CH_DATA* tmpch = g_slist_nth_data(data->prog_info, i);
    gchar* prev_str = string;
    gchar* prog_str = g_strdup_printf(":*:%d^:%s^:%s^:%d",
				 tmpch->number,
				 tmpch->service_name,
				 tmpch->provider_name,
				 tmpch->pids_num);
    for (guint j = 0; j < tmpch->pids_num; j++) {
      ATS_PID_DATA* tmppid = &tmpch->pids[j];
      gchar* prev_prog = prog_str;
      gchar* pid_str = g_strdup_printf("^:%d^:%d^:%s",
				       tmppid->pid,
				       tmppid->type,
				       tmppid->codec);
      prog_str = g_strconcat(prev_prog, pid_str, NULL);
      g_free(pid_str);
      g_free(prev_prog);
    }
    string = g_strconcat(prev_str, prog_str, NULL);
    g_free(prev_str);
    g_free(prog_str);
  }
  return string;
}
