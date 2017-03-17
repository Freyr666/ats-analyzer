#include "parser.hpp"

#define dump_memory_content(desc,spacing)			\
    dump_memory_bytes((desc)->data+2,(desc)->length,spacing)

using namespace Ats;

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
dump_pmt (GstMpegtsSection *section,
	  Metadata         &data)
{
    const GstMpegtsPMT  *pmt = gst_mpegts_section_get_pmt (section);
    GstMpegtsPMTStream  *stream;
    const gchar         *type;
  
    auto channel = data.find_channel(pmt->program_number);
    
    if (channel == nullptr) {
	data.append_channel(Meta_channel(pmt->program_number));
	channel = data.find_channel(pmt->program_number);
    }
  
    for (uint i = 0; i < pmt->streams->len; i++) {
	stream = (GstMpegtsPMTStream*) g_ptr_array_index (pmt->streams, i);
	if (stream->stream_type == 0x86) continue; /* Unknown type */
    
	/* Getting pid's codec type */
	type = enum_name (GST_TYPE_MPEGTS_STREAM_TYPE, stream->stream_type);
	if (type[0] != 'a' && type[0] != 'v') continue;

	auto pid = channel->find_pid(stream->pid);

	if (pid == nullptr) {
	    channel->append_pid(Meta_pid(stream->pid, stream->stream_type, type));
	} else {
	    pid->stream_type = stream->stream_type;
	    pid->stream_type_name = type;
	}
    }
}

static void
dump_pat (GstMpegtsSection *section,
	  Metadata         &data)
{
    GPtrArray           *pat = gst_mpegts_section_get_pat (section);
    GstMpegtsPatProgram *patp;
  
    for (uint i = 0; i < pat->len; i++) {
	patp = (GstMpegtsPatProgram*) g_ptr_array_index (pat, i);
	if (patp->program_number == 0) continue;

	auto channel = data.find_channel(patp->program_number);

	if (channel == nullptr) data.append_channel(Meta_channel(patp->program_number)); 
    }
    g_ptr_array_unref (pat);
}

static void
dump_sdt (GstMpegtsSection *section,
	  Metadata         &data)
{
    const GstMpegtsSDT      *sdt = gst_mpegts_section_get_sdt (section);
    GstMpegtsSDTService     *service;
    GstMpegtsDescriptor     *desc;
    GstMpegtsDVBServiceType service_type;
    char                    *service_name, *provider_name;
  
    g_assert (sdt);
  
    for (uint i = 0; i < sdt->services->len; i++) {
	
	service = (GstMpegtsSDTService*) g_ptr_array_index (sdt->services, i);
	if (service->service_id == 0) continue;
    
	auto channel = data.find_channel(service->service_id);

	if (channel == nullptr) {
	    data.append_channel(Meta_channel(service->service_id));
	    channel = data.find_channel(service->service_id);
	}
    
	for (uint j = 0; j < service->descriptors->len; j++) {
	    desc = (GstMpegtsDescriptor*) g_ptr_array_index (service->descriptors, j);
      
	    if ((desc->tag == GST_MTS_DESC_DVB_SERVICE) &&
		(gst_mpegts_descriptor_parse_dvb_service (desc, &service_type,
							  &service_name, &provider_name))) {
		channel->service_name = service_name;
		channel->provider_name = provider_name;
		g_free (service_name);
		g_free (provider_name);
	    }
	}
    }
}

bool
Parse::table (GstMpegtsSection *section,
	      Metadata         &data)
{
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
	dump_pat (section, data);
	return TRUE;
    case GST_MPEGTS_SECTION_PMT:
	dump_pmt(section, data);
	return TRUE;
    case GST_MPEGTS_SECTION_SDT:
	dump_sdt (section, data);
	return TRUE;
    default:
	return FALSE;
    }
}

bool
Parse::sdt (GstMpegtsSection *section,
	    Metadata         &data)
{
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
	dump_sdt (section, data);
	return TRUE;
    }
    return FALSE;
}

/*
bool
Parse::scte(GstMpegtsSection * section,
	    SIT*               data)
{
    const GstMpegtsSIT *sit;

    if (GST_MPEGTS_SECTION_TYPE (section) == GST_MPEGTS_SECTION_SPLICE_INFO) {
    
	sit = gst_mpegts_section_get_sit (section);
	data->pmt_pid     = sit->pmt_pid;
	data->splice_time = sit->splice_time;
	data->ad          = sit->out_of_netw_ind;
	return TRUE;
    }
    return FALSE;
}
*/
