use gst::prelude::*;
use gst_mpegts_sys::*;
use glib_sys::*;
use gobject_sys::*;
use metadata::{Channel,Pid,Structure};
use std::slice;
use std::str::FromStr;
use std::mem;
use std::ptr;
use std::ffi;
use libc;


fn stream_name (val: i32) -> &'static str {

    match val {
        GST_MPEGTS_STREAM_TYPE_VIDEO_MPEG1         => "Video/Mpeg1",
        GST_MPEGTS_STREAM_TYPE_VIDEO_MPEG2         => "Video/Mpeg2",
        GST_MPEGTS_STREAM_TYPE_AUDIO_MPEG1         => "Audio/Mpeg1",
        GST_MPEGTS_STREAM_TYPE_AUDIO_MPEG2         => "Audio/Mpeg1",
        GST_MPEGTS_STREAM_TYPE_PRIVATE_SECTIONS    => "Private/Sections",
        GST_MPEGTS_STREAM_TYPE_PRIVATE_PES_PACKETS => "Private/Pes/Packets",
        GST_MPEGTS_STREAM_TYPE_MHEG                => "Mheg",
        GST_MPEGTS_STREAM_TYPE_DSM_CC => "Dsm/Cc",
        GST_MPEGTS_STREAM_TYPE_H_222_1 => "H/222/1",
        GST_MPEGTS_STREAM_TYPE_DSMCC_A => "Dsmcc/A",
        GST_MPEGTS_STREAM_TYPE_DSMCC_B => "Dsmcc/B",
        GST_MPEGTS_STREAM_TYPE_DSMCC_C => "Dsmcc/C",
        GST_MPEGTS_STREAM_TYPE_DSMCC_D => "Dsmcc/D",
        GST_MPEGTS_STREAM_TYPE_AUXILIARY => "Auxiliary",
        GST_MPEGTS_STREAM_TYPE_AUDIO_AAC_ADTS => "Audio/Aac_adts",
        GST_MPEGTS_STREAM_TYPE_VIDEO_MPEG4 => "Video/Mpeg4",
        GST_MPEGTS_STREAM_TYPE_AUDIO_AAC_LATM => "Audio/Aac_latm",
        GST_MPEGTS_STREAM_TYPE_SL_FLEXMUX_PES_PACKETS => "Sl/Flexmux_pes_packets",
        GST_MPEGTS_STREAM_TYPE_SL_FLEXMUX_SECTIONS => "Sl/Flexmux_sections",
        GST_MPEGTS_STREAM_TYPE_SYNCHRONIZED_DOWNLOAD => "Synchronized/Download",
        GST_MPEGTS_STREAM_TYPE_METADATA_PES_PACKETS => "Metadata/Pes_packets",
        GST_MPEGTS_STREAM_TYPE_METADATA_SECTIONS => "Metadata/Sections",
        GST_MPEGTS_STREAM_TYPE_METADATA_DATA_CAROUSEL => "Metadata/Data_carousel",
        GST_MPEGTS_STREAM_TYPE_METADATA_OBJECT_CAROUSEL => "Metadata/Object_carousel",
        GST_MPEGTS_STREAM_TYPE_METADATA_SYNCHRONIZED_DOWNLOAD => "Metadata/Synchronized_download",
        GST_MPEGTS_STREAM_TYPE_MPEG2_IPMP => "Mpeg2/Ipmp",
        GST_MPEGTS_STREAM_TYPE_VIDEO_H264 => "Video/H264",
        GST_MPEGTS_STREAM_TYPE_AUDIO_AAC_CLEAN => "Audio/Aac_clean",
        GST_MPEGTS_STREAM_TYPE_MPEG4_TIMED_TEXT => "Mpeg4/Timed_text",
        GST_MPEGTS_STREAM_TYPE_VIDEO_RVC => "Video/Rvc",
        GST_MPEGTS_STREAM_TYPE_VIDEO_H264_SVC_SUB_BITSTREAM => "Video/H264_svc_sub_bitstream",
        GST_MPEGTS_STREAM_TYPE_VIDEO_H264_MVC_SUB_BITSTREAM => "Video/H264_mvc_sub_bitstream",
        GST_MPEGTS_STREAM_TYPE_VIDEO_JP2K => "Video/Jp2k",
        GST_MPEGTS_STREAM_TYPE_VIDEO_MPEG2_STEREO_ADDITIONAL_VIEW => "Video/Mpeg2_stereo_additional_view",
        GST_MPEGTS_STREAM_TYPE_VIDEO_H264_STEREO_ADDITIONAL_VIEW => "Video/H264_stereo_additional_view",
        GST_MPEGTS_STREAM_TYPE_VIDEO_HEVC => "Video/Hevc",
        GST_MPEGTS_STREAM_TYPE_IPMP_STREAM => "Ipmp/Stream",
        _ => "Private/UNKNOWN"
    }
}

unsafe fn dump_pat (section: *mut GstMpegtsSection, metadata: &mut Structure) {
    let pat = gst_mpegts_section_get_pat (section);

    if pat.is_null() { panic!("pat table is empty") }

    let sz = (*pat).len as usize;
    let patp = slice::from_raw_parts((*pat).pdata, sz);

    for p in 0..sz {
        let prog = patp[p] as *mut GstMpegtsPatProgram;

        if prog.is_null() { break }

        let num = (*prog).program_number as u32;
        
        if num == 0 { continue }

        if metadata.channel_exists (num) {
            metadata.add_channel(Channel::new_empty(num));
        }
    }
}

unsafe fn dump_pmt (section: *mut GstMpegtsSection, metadata: &mut Structure) {
    let pmt = gst_mpegts_section_get_pmt (section);

    if pmt.is_null() { panic!("pmt table is empty") }

    let channel = if metadata.channel_exists ((*pmt).program_number as u32) {
        metadata.find_channel_mut_unsafe((*pmt).program_number as u32)
    } else {
        // TODO check if channel be unique
        metadata.add_channel(Channel::new_empty((*pmt).program_number as u32));
        metadata.find_channel_mut_unsafe((*pmt).program_number as u32)
    };

    let sz = (*(*pmt).streams).len as usize;
    let streams = slice::from_raw_parts((*(*pmt).streams).pdata, sz);

    for s in 0..sz {
        let stream = streams[s] as *mut GstMpegtsPMTStream;

        if stream.is_null() { break }
        if (*stream).stream_type == 0x86 { continue } /* Unknown type */
        let pid = (*stream).pid as u32;
        let stream_type = (*stream).stream_type as u32;

        /* Getting pid's codec type */
        let stream_type_name = String::from_str(stream_name (stream_type as i32)).unwrap();

        if channel.pid_exists(pid) {
            let pid = channel.find_pid_mut(pid).unwrap();
            pid.stream_type = stream_type;
            pid.stream_type_name = stream_type_name;
        } else {
            channel.append_pid(Pid::new((*stream).pid as u32, (*stream).stream_type as u32, stream_type_name));
        };
    }
}

unsafe fn dump_sdt (section: *mut GstMpegtsSection, metadata: &mut Structure) {
    let sdt = gst_mpegts_section_get_sdt (section);

    if sdt.is_null() { panic!("sdt table is empty") }
    
    let sz = (*(*sdt).services).len as usize;
    let services = slice::from_raw_parts((*(*sdt).services).pdata, sz);
    
    for c in 0..sz {
        let service = services[c] as *mut GstMpegtsSDTService;

        if service.is_null() { break }
        
        if (*service).service_id == 0 { continue }

        let service_id = (*service).service_id as u32;

        let channel = if metadata.channel_exists (service_id) {
            metadata.find_channel_mut_unsafe(service_id)
        } else {
            // TODO check if channel be unique
            metadata.add_channel(Channel::new_empty(service_id));
            metadata.find_channel_mut_unsafe(service_id)
        };

        let desc_sz = (*(*service).descriptors).len as usize;
        let descriptors = slice::from_raw_parts((*(*service).descriptors).pdata, desc_sz);

        for d in 0..desc_sz {
            let desc = descriptors[d] as *mut GstMpegtsDescriptor;

            if desc.is_null() { break }

            if (*desc).tag as i32 == GST_MTS_DESC_DVB_SERVICE {

                let mut service_name_c : *mut libc::c_char = ptr::null_mut();
                let mut provider_name_c : *mut libc::c_char = ptr::null_mut();
                let mut service_type : GstMpegtsDVBServiceType = 0;
                let serv = gst_mpegts_descriptor_parse_dvb_service(desc, &mut service_type,
                                                                   &mut service_name_c, &mut provider_name_c);

                let service_name = ffi::CStr::from_ptr(service_name_c);
                let provider_name = ffi::CStr::from_ptr(provider_name_c);

                if serv != 0 {
                    channel.service_name = String::from_str(service_name.to_str().unwrap()).unwrap();
                    channel.provider_name = String::from_str(provider_name.to_str().unwrap()).unwrap();
                }
            }
        }
    }
}

pub unsafe fn table (section: *mut GstMpegtsSection, metadata: &mut Structure) -> Option<Structure> {
    if section.is_null() { return None }
    
    match (*section).section_type {
        GST_MPEGTS_SECTION_PAT => { dump_pat(section, metadata);
                                    return Some (metadata.clone()) },
        GST_MPEGTS_SECTION_PMT => { dump_pmt(section, metadata);
                                    return Some (metadata.clone()) },
        GST_MPEGTS_SECTION_SDT => { dump_sdt(section, metadata);
                                    return Some (metadata.clone()) },
        _ => return None
    }
}
