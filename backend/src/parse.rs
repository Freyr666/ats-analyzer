use gst_mpegts_sys::*;
use metadata::{Channel,Pid,Structure};
use std::slice;
use std::str::FromStr;
use std::ptr;
use std::ffi;
use libc;


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

unsafe fn dump_pat (section: *mut GstMpegtsSection, metadata: &mut Structure) {
    let pat = gst_mpegts_section_get_pat (section);

    if pat.is_null() { panic!("pat table is empty") }

    let sz = (*pat).len as usize;
    let patp = slice::from_raw_parts((*pat).pdata, sz);

    for p in patp.iter().take(sz) {
        let prog = *p as *mut GstMpegtsPatProgram;

        if prog.is_null() { break }

        let num = u32::from((*prog).program_number);
        
        if num == 0 { continue }

        if metadata.channel_exists (num) {
            metadata.add_channel(Channel::new_empty(num));
        }
    }
}

unsafe fn dump_pmt (section: *mut GstMpegtsSection, metadata: &mut Structure) {
    let pmt = gst_mpegts_section_get_pmt (section);

    if pmt.is_null() { panic!("pmt table is empty") }

    let channel = if metadata.channel_exists (u32::from((*pmt).program_number)) {
        metadata.find_channel_mut_unsafe(u32::from((*pmt).program_number))
    } else {
        // TODO check if channel be unique
        metadata.add_channel(Channel::new_empty(u32::from((*pmt).program_number)));
        metadata.find_channel_mut_unsafe(u32::from((*pmt).program_number))
    };

    let sz = (*(*pmt).streams).len as usize;
    let streams = slice::from_raw_parts((*(*pmt).streams).pdata, sz);

    for s in streams.iter().take(sz) {
        let stream = *s as *mut GstMpegtsPMTStream;

        if stream.is_null() { break }
        if (*stream).stream_type == 0x86 { continue } /* Unknown type */
        let pid = u32::from((*stream).pid);
        let stream_type = u32::from((*stream).stream_type);

        /* Getting pid's codec type */
        let stream_type_name = String::from_str(stream_name (stream_type as i32)).unwrap();

        if channel.pid_exists(pid) {
            let pid = channel.find_pid_mut(pid).unwrap();
            pid.stream_type = stream_type;
            pid.stream_type_name = stream_type_name;
        } else {
            channel.append_pid(Pid::new(u32::from((*stream).pid),
                                        u32::from((*stream).stream_type),
                                        stream_type_name));
        };
    }
}

unsafe fn dump_sdt (section: *mut GstMpegtsSection, metadata: &mut Structure) {
    let sdt = gst_mpegts_section_get_sdt (section);

    if sdt.is_null() { panic!("sdt table is empty") }
    
    let sz = (*(*sdt).services).len as usize;
    let services = slice::from_raw_parts((*(*sdt).services).pdata, sz);
    
    for c in services.iter().take(sz) {
        let service = *c as *mut GstMpegtsSDTService;

        if service.is_null() { break }
        
        if (*service).service_id == 0 { continue }

        let service_id = u32::from((*service).service_id);

        let channel = if metadata.channel_exists (service_id) {
            metadata.find_channel_mut_unsafe(service_id)
        } else {
            // TODO check if channel be unique
            metadata.add_channel(Channel::new_empty(service_id));
            metadata.find_channel_mut_unsafe(service_id)
        };

        let desc_sz = (*(*service).descriptors).len as usize;
        let descriptors = slice::from_raw_parts((*(*service).descriptors).pdata, desc_sz);

        for d in descriptors.iter().take(desc_sz) {
            let desc = *d as *mut GstMpegtsDescriptor;

            if desc.is_null() { break }

            if i32::from((*desc).tag) == GST_MTS_DESC_DVB_SERVICE {

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
                                    Some (metadata.clone()) },
        GST_MPEGTS_SECTION_PMT => { dump_pmt(section, metadata);
                                    Some (metadata.clone()) },
        GST_MPEGTS_SECTION_SDT => { dump_sdt(section, metadata);
                                    Some (metadata.clone()) },
        _ => None
    }
}
