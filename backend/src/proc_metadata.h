#ifndef PROC_METADATA_H
#define PROC_METADATA_H

#include <glib.h>

#define MAX_PID_NUM 32
#define PID_TYPE_VIDEO 0
#define PID_TYPE_AUDIO 1
#define PID_TYPE_DATA 2

typedef struct __proc_pid_data
{
  guint pid;
  guint type;
  gchar* codec;
  gboolean to_be_analyzed;
} PROC_PID_DATA;

typedef struct __proc_ch_data
{
  guint number;
  gchar* service_name;
  gchar* provider_name;
  guint pids_num;
  PROC_PID_DATA pids[MAX_PID_NUM];
  /* backend properties */
  guint xid;
  gboolean to_be_analyzed;
} PROC_CH_DATA;

typedef struct __proc_metadata
{
  guint stream_id;
  GSList* prog_info; /* PROC_CH_DATA */
  /* true if metadata has been updated by backend */
  gboolean done;
} PROC_METADATA;

PROC_CH_DATA* proc_metadata_find_channel(PROC_METADATA* data, guint num);

gboolean proc_metadata_is_ready(PROC_METADATA* data);

void proc_metadata_print(PROC_METADATA* data);

#endif /* PROC_METADATA_H */
