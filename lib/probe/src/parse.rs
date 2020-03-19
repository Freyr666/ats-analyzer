use gst_mpegts_sys::*;
use media_stream::{Pid,Structure};
use std::slice;
use std::str::FromStr;
use std::ptr;
use std::ffi;
use libc;
use std::collections::HashMap;

fn stream_name (val: i32) -> &'static str {

    match val {
        GST_MPEGTS_STREAM_TYPE_VIDEO_MPEG1         => "video/mpeg1",
        GST_MPEGTS_STREAM_TYPE_VIDEO_MPEG2         => "video/mpeg2",
        GST_MPEGTS_STREAM_TYPE_AUDIO_MPEG1         => "audio/mpeg1",
        GST_MPEGTS_STREAM_TYPE_AUDIO_MPEG2         => "audio/mpeg1",
        GST_MPEGTS_STREAM_TYPE_PRIVATE_SECTIONS    => "private/sections",
        GST_MPEGTS_STREAM_TYPE_PRIVATE_PES_PACKETS => "private/pes_packets",
        GST_MPEGTS_STREAM_TYPE_MHEG                => "mheg",
/*      GST_MPEGTS_STREAM_TYPE_DSM_CC => "dsm/cc",
        GST_MPEGTS_STREAM_TYPE_H_222_1 => "h222/1",
        GST_MPEGTS_STREAM_TYPE_DSMCC_A => "dsmcc/A",
        GST_MPEGTS_STREAM_TYPE_DSMCC_B => "Dsmcc/B",
        GST_MPEGTS_STREAM_TYPE_DSMCC_C => "Dsmcc/C",
        GST_MPEGTS_STREAM_TYPE_DSMCC_D => "Dsmcc/D", */
        GST_MPEGTS_STREAM_TYPE_AUXILIARY => "auxiliary",
        GST_MPEGTS_STREAM_TYPE_AUDIO_AAC_ADTS => "audio/aac_adts",
        GST_MPEGTS_STREAM_TYPE_VIDEO_MPEG4 => "video/mpeg4",
        GST_MPEGTS_STREAM_TYPE_AUDIO_AAC_LATM => "audio/aac_latm",
        GST_MPEGTS_STREAM_TYPE_SL_FLEXMUX_PES_PACKETS => "sl_flexmux/pes_packets",
        GST_MPEGTS_STREAM_TYPE_SL_FLEXMUX_SECTIONS => "sl_flexmux/sections",
        GST_MPEGTS_STREAM_TYPE_SYNCHRONIZED_DOWNLOAD => "synchronized_download",
        GST_MPEGTS_STREAM_TYPE_METADATA_PES_PACKETS => "metadata/pes_packets",
        GST_MPEGTS_STREAM_TYPE_METADATA_SECTIONS => "metadata/sections",
        GST_MPEGTS_STREAM_TYPE_METADATA_DATA_CAROUSEL => "metadata/data_carousel",
        GST_MPEGTS_STREAM_TYPE_METADATA_OBJECT_CAROUSEL => "metadata/object_carousel",
        GST_MPEGTS_STREAM_TYPE_METADATA_SYNCHRONIZED_DOWNLOAD => "metadata/synchronized_download",
        GST_MPEGTS_STREAM_TYPE_MPEG2_IPMP => "mpeg2/ipmp",
        GST_MPEGTS_STREAM_TYPE_VIDEO_H264 => "video/h264",
        GST_MPEGTS_STREAM_TYPE_AUDIO_AAC_CLEAN => "audio/aac_clean",
        GST_MPEGTS_STREAM_TYPE_MPEG4_TIMED_TEXT => "mpeg4/timed_text",
        GST_MPEGTS_STREAM_TYPE_VIDEO_RVC => "video/rvc",
        GST_MPEGTS_STREAM_TYPE_VIDEO_H264_SVC_SUB_BITSTREAM => "video/h264_svc_sub_bitstream",
        GST_MPEGTS_STREAM_TYPE_VIDEO_H264_MVC_SUB_BITSTREAM => "video/h264_mvc_sub_bitstream",
        GST_MPEGTS_STREAM_TYPE_VIDEO_JP2K => "video/jp2k",
        GST_MPEGTS_STREAM_TYPE_VIDEO_MPEG2_STEREO_ADDITIONAL_VIEW => "video/mpeg2_stereo_additional_view",
        GST_MPEGTS_STREAM_TYPE_VIDEO_H264_STEREO_ADDITIONAL_VIEW => "video/h264_stereo_additional_view",
        GST_MPEGTS_STREAM_TYPE_VIDEO_HEVC => "video/hevc",
        GST_MPEGTS_STREAM_TYPE_IPMP_STREAM => "ipmp/stream",
        _ => "private/UNKNOWN"
    }
}

unsafe fn dump_pat (section: *mut GstMpegtsSection,
                    name_table: &mut HashMap<u32,(String,String)>,
                    metadata: &mut Structure) {
    let pat = gst_mpegts_section_get_pat (section);

    if pat.is_null() {
        //warn!("pat table is empty");
        return
    }

    metadata.pids = vec![];
    name_table.clear();
}

unsafe fn dump_pmt (section: *mut GstMpegtsSection,
                    name_table: &mut HashMap<u32,(String,String)>,
                    metadata: &mut Structure) {
    let pmt = gst_mpegts_section_get_pmt (section);

    if pmt.is_null() {
        //warn!("pmt table is empty");
        return
    }

    let group_id = u32::from((*pmt).program_number);

    let sz = (*(*pmt).streams).len as usize;
    let streams = slice::from_raw_parts((*(*pmt).streams).pdata, sz);

    for s in streams.iter().take(sz) {
        let stream = *s as *mut GstMpegtsPMTStream;

        if stream.is_null() { break }
        if (*stream).stream_type == 0x86 { continue } /* Unknown type */
        let id = u32::from((*stream).pid);
        let stream_type = u32::from((*stream).stream_type);

        /* Getting pid's codec type */
        let stream_type_name = String::from_str(stream_name (stream_type as i32)).unwrap();

        let names = name_table.get (&group_id);
        
        /* Update pid if exists */
        if let Some (pid) = metadata.find_pid_mut (id, Some (group_id)) {
            pid.stream_type = stream_type;
            pid.stream_type_name = stream_type_name;

            if let Some ((service_name, provider_name)) = names {
                pid.service_name = service_name.clone();
                pid.provider_name = provider_name.clone();
            };
            return;
        };
        /* Or else add a new pid */
        let (service_name, provider_name) = match names {
            Some ((x,y)) => (x.clone(), y.clone()),
            None => (String::from(""), String::from(""))
        };
        let pid = Pid::new(id,
                           Some (group_id),
                           service_name,
                           provider_name,
                           stream_type,
                           stream_type_name);
        metadata.add_pid (pid);
    }
}

unsafe fn dump_sdt (section: *mut GstMpegtsSection,
                    name_table: &mut HashMap<u32,(String,String)>,
                    metadata: &mut Structure) {
    let sdt = gst_mpegts_section_get_sdt (section);

    if sdt.is_null() {
        //warn!("sdt table is empty");
        return
    }
    
    let sz = (*(*sdt).services).len as usize;
    let services = slice::from_raw_parts((*(*sdt).services).pdata, sz);
    
    for c in services.iter().take(sz) {
        let service = *c as *mut GstMpegtsSDTService;

        if service.is_null() { break }
        
        if (*service).service_id == 0 { continue }

        let group_id = u32::from((*service).service_id);

        let desc_sz = (*(*service).descriptors).len as usize;
        let descriptors = slice::from_raw_parts((*(*service).descriptors).pdata, desc_sz);

        for d in descriptors.iter().take(desc_sz) {
            let desc = *d as *mut GstMpegtsDescriptor;

            if desc.is_null() { break }

            if i32::from((*desc).tag) == GST_MTS_DESC_DVB_SERVICE {

                let mut service_name_c : *mut libc::c_char = ptr::null_mut();
                let mut provider_name_c : *mut libc::c_char = ptr::null_mut();
                let mut service_type : GstMpegtsDVBServiceType = 0;
                let serv = gst_mpegts_descriptor_parse_dvb_service(desc,
                                                                   &mut service_type,
                                                                   &mut service_name_c,
                                                                   &mut provider_name_c);

                let service_name = ffi::CStr::from_ptr(service_name_c);
                let provider_name = ffi::CStr::from_ptr(provider_name_c);
                let service_name =
                    String::from_str(service_name.to_str().unwrap())
                    .unwrap();
                let provider_name =
                    String::from_str(provider_name.to_str().unwrap())
                    .unwrap();
                
                /* Update existing pids */
                if serv != 0 {
                    metadata.map_group_mut (Some (group_id), |p| {
                        p.service_name = service_name.clone();
                        p.provider_name = provider_name.clone();
                    });
                }

                name_table.insert (group_id,
                                   (service_name, provider_name));
            }
        }
    }
}

pub unsafe fn table (section: *mut GstMpegtsSection,
                     name_table: &mut HashMap<u32,(String,String)>,
                     metadata: &mut Structure) -> Option<Structure> {
    if section.is_null() { return None }
    
    match (*section).section_type {
        GST_MPEGTS_SECTION_PAT => { dump_pat(section, name_table, metadata);
                                    Some (metadata.clone()) },
        GST_MPEGTS_SECTION_PMT => { dump_pmt(section, name_table, metadata);
                                    Some (metadata.clone()) },
        GST_MPEGTS_SECTION_SDT => { dump_sdt(section, name_table, metadata);
                                    Some (metadata.clone()) },
        _ => None
    }
}
