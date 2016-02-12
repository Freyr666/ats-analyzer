#include "parse_ts.h"
#include "ats_branch.h"
#include "ats_metadata.h"
#include "ats_tree.h"
#include <string.h>


#define dump_memory_content(desc, spacing) dump_memory_bytes((desc)->data + 2, (desc)->length, spacing)


static const gchar *
enum_name (GType instance_type, gint val)
{
  GEnumValue *en;

  en = g_enum_get_value (G_ENUM_CLASS (g_type_class_peek (instance_type)), val);

  if (!en)
    return "UNKNOWN/PRIVATE";
  return en->value_nick;
}

static void
dump_pmt (GstMpegtsSection * section, ATS_METADATA* data)
{
  const GstMpegtsPMT *pmt = gst_mpegts_section_get_pmt (section);
  guint i, len;
  ATS_CH_DATA* channel;
  
  channel = ats_metadata_find_channel(data, pmt->program_number);
  if (!channel) return;
  len = pmt->streams->len;
  channel->pids_num = 0;
  for (i = 0; i < len; i++) {
    GstMpegtsPMTStream *stream = g_ptr_array_index (pmt->streams, i);
    if (stream->stream_type == 0x86) continue; /* Unknown type */
    channel->pids[channel->pids_num].to_be_analyzed = TRUE;
    channel->pids[channel->pids_num].pid = stream->pid;
    channel->pids[channel->pids_num].type = stream->stream_type;
    channel->pids[channel->pids_num].codec = g_strdup(enum_name (GST_TYPE_MPEGTS_STREAM_TYPE, stream->stream_type));
    channel->pids_num++;
  }
}

static void
dump_pat (GstMpegtsSection * section, ATS_METADATA* data)
{
  GPtrArray *pat = gst_mpegts_section_get_pat (section);
  guint i, len;

  len = pat->len;
  if ((data == NULL) || (data->prog_info != NULL)) return;
  for (i = 0; i < len; i++) {
    GstMpegtsPatProgram *patp = g_ptr_array_index (pat, i);
    if (patp->program_number == 0) continue;
    ATS_CH_DATA* tmpch = g_new(ATS_CH_DATA, 1);
    tmpch->number = patp->program_number;
    tmpch->xid = 0;
    tmpch->pids_num = 0;
    tmpch->to_be_analyzed = 1;
    tmpch->provider_name = NULL;
    tmpch->service_name = NULL;
    data->prog_info = g_slist_append(data->prog_info, tmpch);   
  }
  g_ptr_array_unref (pat);
}

static void
dump_sdt (GstMpegtsSection * section, ATS_METADATA* data)
{
  const GstMpegtsSDT *sdt = gst_mpegts_section_get_sdt (section);
  guint i, len;

  g_assert (sdt);

  len = sdt->services->len;
  if ((data == NULL) || (data->prog_info == NULL)) return;
  for (i = 0; i < len; i++) {
    GstMpegtsSDTService *service = g_ptr_array_index (sdt->services, i);
    if (service->service_id == 0) continue;
    ATS_CH_DATA* tmpch = ats_metadata_find_channel(data, service->service_id);
    if (!tmpch) continue;
    for (guint i = 0; i < service->descriptors->len; i++) {
      GstMpegtsDescriptor *desc = g_ptr_array_index (service->descriptors, i);
      if (desc->tag == GST_MTS_DESC_DVB_SERVICE) {
	gchar *service_name, *provider_name;
	GstMpegtsDVBServiceType service_type;
	if (gst_mpegts_descriptor_parse_dvb_service (desc, &service_type,
						     &service_name, &provider_name)) {
	  tmpch->service_name = g_strdup(service_name);
	  tmpch->provider_name = g_strdup(provider_name);
	  g_free (service_name);
	  g_free (provider_name);
	}
      }
    }
  }
  data->done = TRUE;
}

gboolean
parse_table (GstMpegtsSection * section, void* data)
{
  ATS_METADATA* metadata; 
  metadata = (ATS_METADATA*)data;

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
    dump_pat (section, metadata);
    return TRUE;
    break;
  case GST_MPEGTS_SECTION_PMT:
    dump_pmt(section, data);
    return TRUE;
    break;
  case GST_MPEGTS_SECTION_SDT:
    dump_sdt (section, metadata);
    return TRUE;
    break;
  default:
    break;
  }
  return FALSE;
}
