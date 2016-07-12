#include "ats_metadata.h"

#include <stdio.h>

static void
delete_node(gpointer node)
{
  ATS_CH_DATA* data;
  guint        i;

  data = node;
  if (data->provider_name) g_free(data->provider_name);
  if (data->service_name)  g_free(data->service_name);
  for (i = 0; i < data->pids_num; i++) {
    if (data->pids[i].codec) g_free(data->pids[i].codec);
  }
}

ATS_METADATA*
ats_metadata_new(guint stream_id)
{
  ATS_METADATA* rval;

  rval = g_new(ATS_METADATA, 1);
  rval->stream_id = stream_id;
  rval->prog_info = NULL;
  return rval;
}

void
ats_metadata_delete(ATS_METADATA* this)
{
  g_slist_free_full(this->prog_info, delete_node);
  g_free(this);
  this = NULL;
}

void
ats_metadata_reset(ATS_METADATA* this)
{
  ATS_CH_DATA* tmpch;
  GSList*      elem;
  guint        j;
  
  for (elem = this->prog_info; elem; elem = elem->next) {
    tmpch = elem->data;
    tmpch->to_be_analyzed = FALSE;
    tmpch->xid = 0;
    for (j = 0; j < tmpch->pids_num; j++) {
      tmpch->pids[j].to_be_analyzed = FALSE;
    }
  }
}

ATS_CH_DATA*
ats_metadata_find_channel(ATS_METADATA* this, guint num)
{
  ATS_CH_DATA* data;
  GSList*      elem;
  
  for (elem = this->prog_info; elem; elem = elem->next) {
    data = elem->data;
    if (data->number == num) return data;
  }
  return NULL;
}

ATS_PID_DATA*
ats_metadata_find_pid(ATS_METADATA* data, guint ch, guint pid)
{
  ATS_PID_DATA* rval;
  ATS_CH_DATA*  tmpch;
  guint         i;

  tmpch = ats_metadata_find_channel(data, ch);
  if (tmpch == NULL) return NULL;
  
  for (i = 0; i < tmpch->pids_num; i++) {
    rval = &tmpch->pids[i];
    if (rval->pid == pid) return rval;
  }
  return NULL;
}

gboolean
ats_metadata_are_ready(const ATS_METADATA* data)
{
  GSList*      elem;
  ATS_CH_DATA* ch;
  
  if (data == NULL) return FALSE;
  if (data->prog_info == NULL) return FALSE;
  
  for (elem = data->prog_info; elem; elem = elem->next) {
    ch = elem->data;
    if (ch->pids_num == 0) return FALSE;
  }
  return TRUE;
}

gchar*
ats_metadata_to_string(const ATS_METADATA* data)
{
  GSList*       elem;
  ATS_CH_DATA*  ch;
  ATS_PID_DATA* pid;
  gchar*        prev_str;
  gchar*        prog_str;
  gchar*        prev_prog;
  gchar*        pid_str;
  gchar*        string;
  guint         i;
  
  string = g_strdup_printf("d%d", data->stream_id);

  for (elem = data->prog_info; elem; elem = elem->next) {
    ch = elem->data;
    prev_str = string;
    prog_str = g_strdup_printf(":*:%d^:%s^:%s^:%d",
			       ch->number,
			       ch->service_name,
			       ch->provider_name,
			       ch->pids_num);
    for (i = 0; i < ch->pids_num; i++) {
      pid = &ch->pids[i];
      prev_prog = prog_str;
      pid_str = g_strdup_printf("^:%d^:%d^:%s",
				pid->pid,
				pid->type,
				pid->codec);
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
