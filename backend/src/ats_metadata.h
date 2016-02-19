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
} ATS_METADATA;

ATS_METADATA* ats_metadata_new(guint stream_id);

void ats_metadata_delete(ATS_METADATA* this);

void ats_metadata_reset(ATS_METADATA* this);

ATS_CH_DATA* ats_metadata_find_channel(const ATS_METADATA* this, guint num);

ATS_PID_DATA* ats_metadata_find_pid(const ATS_METADATA* data, guint ch, guint pid);

#define ats_metadata_ch_number(data)((data->prog_info == NULL)?0:g_slist_length(data->prog_info))

#define ats_metadata_got_sdt(data)\
  (((ATS_CH_DATA*)g_slist_nth_data(data->prog_info,0))->service_name != NULL)

gboolean ats_metadata_is_ready(const ATS_METADATA* data);

gchar* ats_metadata_to_string(const ATS_METADATA* data);

#endif /* ATS_METADATA_H */
