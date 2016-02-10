#include "proc_metadata.h"

#include <stdio.h>

PROC_CH_DATA*
proc_metadata_find_channel(PROC_METADATA* data, guint num)
{
  PROC_CH_DATA* rval;
  guint len;
  if (data->prog_info == NULL) return NULL;
  len = g_slist_length(data->prog_info);
  for (guint i = 0; i < len; i++) {
    rval = g_slist_nth_data(data->prog_info, i);
    if (rval->number == num) return rval;
  }
  return NULL;
}

gboolean
proc_metadata_is_ready(PROC_METADATA* data)
{
  guint len;
  if (data == NULL) return FALSE;
  if (data->prog_info == NULL) return FALSE;
  len = g_slist_length(data->prog_info);
  for (guint i = 0; i < len; i++) {
    PROC_CH_DATA* ch = g_slist_nth_data(data->prog_info, i);
    if (ch->pids_num == 0) return FALSE;
  }
  return TRUE;
}

void
proc_metadata_print(PROC_METADATA* data)
{
  printf("id: %d\n", data->stream_id);
  guint len = g_slist_length(data->prog_info);
  for (guint i = 0; i < len; i++) {
    PROC_CH_DATA *ch = g_slist_nth_data(data->prog_info, i);
    printf("num: %d name: %s pids: %d\n", ch->number, ch->service_name, ch->pids_num);
    for (guint j = 0; j < ch->pids_num; j++) {
      printf("\tpid: %d codec: %s type: %d\n", ch->pids[j].pid, ch->pids[j].codec, ch->pids[j].type);
    }
  }
}
