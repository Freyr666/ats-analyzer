#include "parse_ts.h"
#include "proc_branch.h"
#include "proc_metadata.h"
#include "proc_tree.h"


#define dump_memory_content(desc, spacing) dump_memory_bytes((desc)->data + 2, (desc)->length, spacing)

static void
dump_descriptors (GPtrArray * descriptors, PROC_CH_DATA* ch)
{
  guint i;

  for (i = 0; i < descriptors->len; i++) {
    GstMpegtsDescriptor *desc = g_ptr_array_index (descriptors, i);
    switch (desc->tag) {
      case GST_MTS_DESC_DVB_SERVICE:
      {
        gchar *service_name, *provider_name;
        GstMpegtsDVBServiceType service_type;
        if (gst_mpegts_descriptor_parse_dvb_service (desc, &service_type,
                &service_name, &provider_name)) {
	  ch->service_name = g_strdup(service_name);
	  ch->provider_name = g_strdup(provider_name);
          g_free (service_name);
          g_free (provider_name);
        }
        break;
      }
      default:
        break;
    }
  }
}

static gboolean
dump_pat (GstMpegtsSection * section, PROC_METADATA* data)
{
  GPtrArray *pat = gst_mpegts_section_get_pat (section);
  guint i, len;

  len = pat->len;
  if ((data == NULL) || (data->proginfo != NULL)) return FALSE;
  for (i = 0; i < len; i++) {
    GstMpegtsPatProgram *patp = g_ptr_array_index (pat, i);
    if (patp->program_number == 0) continue;
    data->prognum++;
    PROC_CH_DATA* tmpch = g_new(PROC_CH_DATA, 1);
    tmpch->number = patp->program_number;
    tmpch->xid = 0;
    tmpch->to_be_analyzed = 1;
    tmpch->service_id = patp->program_number;;
    tmpch->provider_name = NULL;
    tmpch->service_name = NULL;
    data->proginfo = g_slist_append(data->proginfo, tmpch);   
  }
  g_ptr_array_unref (pat);
  return TRUE;
}

static gboolean
dump_sdt (GstMpegtsSection * section, PROC_METADATA* data)
{
  const GstMpegtsSDT *sdt = gst_mpegts_section_get_sdt (section);
  guint i, len;

  g_assert (sdt);

  len = sdt->services->len;
  if ((data == NULL) || (data->proginfo == NULL)) return FALSE;
  for (i = 0; i < len; i++) {
    GstMpegtsSDTService *service = g_ptr_array_index (sdt->services, i);
    if (service->service_id == 0) continue;
    PROC_CH_DATA* tmpch = proc_metadata_find_channel(data, service->service_id);
    tmpch->service_id = service->service_id;
    dump_descriptors (service->descriptors, tmpch);
  }
  return TRUE;
}

gboolean
parse_table (GstMpegtsSection * section, void* data)
{
  PROC_METADATA* metadata;
  gboolean rval = FALSE; 
  metadata = (PROC_METADATA*)data;

  g_type_class_ref (GST_TYPE_MPEGTS_SECTION_TYPE);
  g_type_class_ref (GST_TYPE_MPEGTS_SECTION_TABLE_ID);
  g_type_class_ref (GST_TYPE_MPEGTS_RUNNING_STATUS);
  g_type_class_ref (GST_TYPE_MPEGTS_DESCRIPTOR_TYPE);
  g_type_class_ref (GST_TYPE_MPEGTS_DVB_DESCRIPTOR_TYPE);
  g_type_class_ref (GST_TYPE_MPEGTS_ATSC_DESCRIPTOR_TYPE);
  g_type_class_ref (GST_TYPE_MPEGTS_ISDB_DESCRIPTOR_TYPE);
  g_type_class_ref (GST_TYPE_MPEGTS_MISC_DESCRIPTOR_TYPE);
  g_type_class_ref (GST_TYPE_MPEGTS_ISO639_AUDIO_TYPE);
  g_type_class_ref (GST_TYPE_MPEGTS_DVB_SERVICE_TYPE);
  g_type_class_ref (GST_TYPE_MPEGTS_DVB_TELETEXT_TYPE);
  g_type_class_ref (GST_TYPE_MPEGTS_STREAM_TYPE);
  g_type_class_ref (GST_TYPE_MPEGTS_SECTION_DVB_TABLE_ID);
  g_type_class_ref (GST_TYPE_MPEGTS_SECTION_ATSC_TABLE_ID);
  g_type_class_ref (GST_TYPE_MPEGTS_SECTION_SCTE_TABLE_ID);
  g_type_class_ref (GST_TYPE_MPEGTS_MODULATION_TYPE);
  g_type_class_ref (GST_TYPE_MPEGTS_DVB_CODE_RATE);
  g_type_class_ref (GST_TYPE_MPEGTS_CABLE_OUTER_FEC_SCHEME);
  g_type_class_ref (GST_TYPE_MPEGTS_TERRESTRIAL_TRANSMISSION_MODE);
  g_type_class_ref (GST_TYPE_MPEGTS_TERRESTRIAL_GUARD_INTERVAL);
  g_type_class_ref (GST_TYPE_MPEGTS_TERRESTRIAL_HIERARCHY);
  g_type_class_ref (GST_TYPE_MPEGTS_DVB_LINKAGE_TYPE);
  g_type_class_ref (GST_TYPE_MPEGTS_DVB_LINKAGE_HAND_OVER_TYPE);
  g_type_class_ref (GST_TYPE_MPEGTS_COMPONENT_STREAM_CONTENT);
  g_type_class_ref (GST_TYPE_MPEGTS_CONTENT_NIBBLE_HI);

  switch (GST_MPEGTS_SECTION_TYPE (section)) {
    case GST_MPEGTS_SECTION_PAT:
      rval = dump_pat (section, metadata);
      break;
    case GST_MPEGTS_SECTION_SDT:
      rval = dump_sdt (section, metadata);
      break;
    default:
      break;
  }
  return rval;
}
