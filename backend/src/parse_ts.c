#include "parse_ts.h"
#include "ats_branch.h"
#include "ats_metadata.h"
#include "ats_tree.h"
#include <string.h>

#define dump_memory_content(desc,spacing)\
  dump_memory_bytes((desc)->data+2,(desc)->length,spacing)


static const gchar *
enum_name (GType instance_type,
	   gint  val)
{
  GEnumValue *en;
  
  en = g_enum_get_value (G_ENUM_CLASS (g_type_class_peek (instance_type)), val);

  if (!en)
    return "UNKNOWN/PRIVATE";
  return en->value_nick;
}

static void
dump_pmt (GstMpegtsSection * section,
	  ATS_METADATA*      data)
{
  const GstMpegtsPMT  *pmt = gst_mpegts_section_get_pmt (section);
  guint               i, len;
  ATS_CH_DATA         *channel;
  GstMpegtsPMTStream  *stream;
  const gchar         *type;
  
  channel = ats_metadata_find_channel(data, pmt->program_number);
  
  if (!channel) return;
  
  len = pmt->streams->len;
  channel->pids_num = 0;
  
  for (i = 0; i < len; i++) {
    stream = g_ptr_array_index (pmt->streams, i);
    if (stream->stream_type == 0x86) continue; /* Unknown type */
    
    /* Getting pid's codec type */
    type = enum_name (GST_TYPE_MPEGTS_STREAM_TYPE, stream->stream_type);
    if (type[0] != 'a' && type[0] != 'v') continue;
    
    channel->pids[channel->pids_num].to_be_analyzed = FALSE;
    channel->pids[channel->pids_num].pid = stream->pid;
    channel->pids[channel->pids_num].type = stream->stream_type;
    channel->pids[channel->pids_num].codec = g_strdup(type);
    channel->pids_num++;
  }
}

static void
dump_pat (GstMpegtsSection * section,
	  ATS_METADATA*      data)
{
  GPtrArray           *pat = gst_mpegts_section_get_pat (section);
  guint                i, len;
  GstMpegtsPatProgram *patp;
  ATS_CH_DATA         *ch;

  len = pat->len;
  if ((data == NULL) || (data->prog_info != NULL)) return;
  
  for (i = 0; i < len; i++) {
    patp = g_ptr_array_index (pat, i);
    if (patp->program_number == 0) continue;
    
    ch = g_new(ATS_CH_DATA, 1);
    
    ch->number = patp->program_number;
    ch->xid = 0;
    ch->pids_num = 0;
    ch->to_be_analyzed = FALSE;
    ch->provider_name = NULL;
    ch->service_name = NULL;
    
    data->prog_info = g_slist_append(data->prog_info, ch);   
  }
  g_ptr_array_unref (pat);
}

static void
dump_sdt (GstMpegtsSection * section,
	  ATS_METADATA*      data)
{
  const GstMpegtsSDT      *sdt = gst_mpegts_section_get_sdt (section);
  guint                   i, j, len;
  GstMpegtsSDTService     *service;
  ATS_CH_DATA             *ch;
  GstMpegtsDescriptor     *desc;
  gchar                   *service_name, *provider_name;
  GstMpegtsDVBServiceType service_type;
  
  g_assert (sdt);

  len = sdt->services->len;
  if ((data == NULL) || (data->prog_info == NULL)) return;
  
  for (i = 0; i < len; i++) {
    service = g_ptr_array_index (sdt->services, i);
    if (service->service_id == 0) continue;
    
    ch = ats_metadata_find_channel(data, service->service_id);
    if (!ch) continue;
    
    for (j = 0; j < service->descriptors->len; j++) {
      desc = g_ptr_array_index (service->descriptors, j);
      
      if ((desc->tag == GST_MTS_DESC_DVB_SERVICE) &&
	  (gst_mpegts_descriptor_parse_dvb_service (desc, &service_type,
						    &service_name, &provider_name))) {
	ch->service_name = g_strdup(service_name);
	ch->provider_name = g_strdup(provider_name);
	g_free (service_name);
	g_free (provider_name);
      }
    }
  }
}

gboolean
parse_table (GstMpegtsSection * section,
	     void*              data)
{
  ATS_METADATA* metadata;
  
  metadata = data;

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
  case GST_MPEGTS_SECTION_PMT:
    dump_pmt(section, metadata);
    return TRUE;
  case GST_MPEGTS_SECTION_SDT:
    dump_sdt (section, metadata);
    return TRUE;
  default:
    return FALSE;
  }
}

gboolean
parse_sdt (GstMpegtsSection * section,
	   void*              data)
{
  ATS_METADATA* metadata;
  
  metadata = data;

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

  if (GST_MPEGTS_SECTION_TYPE (section) == GST_MPEGTS_SECTION_SDT) {
    dump_sdt (section, metadata);
    return TRUE;
  }
  return FALSE;
}

gboolean
parse_scte(GstMpegtsSection * section,
	   void*              data)
{
  return TRUE;
}
