#ifndef ATS_METADATA_H
#define ATS_METADATA_H

#include <glib.h>

#define MAX_PID_NUM 32
#define PID_TYPE_VIDEO 0
#define PID_TYPE_AUDIO 1
#define PID_TYPE_DATA 2

typedef struct __ats_pid_data
{
  guint pid;
  guint type;
  gchar* codec;
  gboolean to_be_analyzed;
} ATS_PID_DATA;

typedef struct __ats_ch_data
{
  guint number;
  gchar* service_name;
  gchar* provider_name;
  guint pids_num;
  ATS_PID_DATA pids[MAX_PID_NUM];
  /* backend properties */
  guint xid;
  gboolean to_be_analyzed;
} ATS_CH_DATA;

typedef struct __ats_metadata
{
  guint stream_id;
  GSList* prog_info; /* ATS_CH_DATA */
  /* true if metadata has been updated by backend */
  gboolean done;
} ATS_METADATA;

ATS_CH_DATA* ats_metadata_find_channel(ATS_METADATA* data, guint num);

gboolean ats_metadata_is_ready(ATS_METADATA* data);

void ats_metadata_print(ATS_METADATA* data);

#endif /* ATS_METADATA_H */
