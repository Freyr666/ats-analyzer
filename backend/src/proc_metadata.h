#ifndef PROC_METADATA_H
#define PROC_METADATA_H

#include <glib.h>

typedef struct __proc_ch_data
{
  guint number;
  guint service_id;
  gchar* service_name;
  gchar* provider_name;
  guint xid;
  gboolean to_be_analyzed;
} PROC_CH_DATA;

typedef struct __proc_metadata
{
  guint prognum;
  GSList* proginfo; /* PROC_CH_DATA */
} PROC_METADATA;

gboolean proc_metadata_is_valid(PROC_METADATA* data);

PROC_CH_DATA* proc_metadata_find_channel(PROC_METADATA* data, guint num);

#endif /* PROC_METADATA_H */
