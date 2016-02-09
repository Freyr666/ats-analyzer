#include "proc_metadata.h"

gboolean
proc_metadata_is_valid(PROC_METADATA* data)
{
  return ((data->prognum != 0) && (data->proginfo != NULL));
}

PROC_CH_DATA*
proc_metadata_find_channel(PROC_METADATA* data, guint num){
  for (guint i = 0; i < data->prognum; i++) {
    PROC_CH_DATA* ch = g_slist_nth_data(data->proginfo, i);
    if (ch->number == num)
      return ch;
  }
  return NULL;
}
