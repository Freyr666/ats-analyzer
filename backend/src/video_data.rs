use gst;
use gst_sys;
use libc;
use gst::prelude::*;
use std::slice;
use std::mem;
use std::sync::mpsc::Sender;
use serde::Deserialize;
use chatterer::notif::Notifier;
use chatterer::MsgType;

#[derive(Serialize,Deserialize,Debug)]
#[repr(C)]
struct Param {
    min: f32,
    max: f32,
    avg: f32,
}

#[derive(Serialize,Deserialize,Debug)]
#[repr(C)]
struct Error {
    counter:   u32,
    size:      u32,
    params:    Param,
    timestamp: i64,
    peak_flag: bool,
    cont_flag: bool,
}

#[repr(C)]
enum Parameter {
    Black,
    Luma,
    Freeze,
    Diff,
    Blocky,
    ParamNumber,
}

#[derive(Serialize,Deserialize,Debug)]
struct Errors<'a> {
    #[serde(bound(deserialize = "&'a Error: Deserialize<'de>"))]
    black:  &'a Error,
    luma:   &'a Error,
    freeze: &'a Error,
    diff:   &'a Error,
    blocky: &'a Error,
} 

#[derive(Serialize,Deserialize,Debug)]
struct Msg<'a> {
    #[serde(bound(deserialize = "&'a String: Deserialize<'de>"))]
    stream:     &'a String,
    channel:    u32,
    pid:        u32,
    #[serde(bound(deserialize = "Errors<'a>: Deserialize<'de>"))]
    errors:     Errors<'a>
}

#[derive(Serialize,Deserialize,Debug)]
struct MsgStatus<'a> {
    #[serde(bound(deserialize = "&'a String: Deserialize<'de>"))]
    stream:     &'a String,
    channel:    u32,
    pid:        u32,
    playing:    bool,
}

pub struct VideoData {
    stream:       String,
    channel:      u32,
    pid:          u32,
    notif:        Notifier,
    notif_status: Notifier,
    mmap:         *mut gst_sys::GstMapInfo,
}

unsafe impl Send for VideoData {}

//unsafe impl Sync for VideoData {}

impl VideoData {

    pub fn new (stream: String, channel: u32, pid: u32, format: MsgType, sender: Sender<Vec<u8>>) -> VideoData {
        let mmap :  *mut gst_sys::GstMapInfo;
        unsafe {
            mmap = libc::malloc(mem::size_of::<gst_sys::GstMapInfo>()) as *mut gst_sys::GstMapInfo;
        }
        let notif = Notifier::new("video_data", format, sender.clone());
        let notif_status = Notifier::new("stream_lost", format, sender);
        VideoData { stream, channel, pid, notif, notif_status, mmap }
    }

    pub fn send_msg (&self, ebuf: &gst::Buffer) {
        unsafe {
            //gst_sys::GstMapFlags::READ
            if gst_sys::gst_buffer_map(ebuf.as_mut_ptr(), self.mmap, 1) == 0 {
                panic!("video_data: ebuf mmap failure");
            }
            let pointer: *const Error = (*self.mmap).data as *const Error;
            let err_buf = slice::from_raw_parts(pointer, Parameter::ParamNumber as usize);

            let errors: Errors = Errors {
                black:  &err_buf[Parameter::Black as usize],
                luma:   &err_buf[Parameter::Luma as usize],
                freeze: &err_buf[Parameter::Freeze as usize],
                diff:   &err_buf[Parameter::Diff as usize],
                blocky: &err_buf[Parameter::Blocky as usize],
            };

            let msg = Msg { stream: &self.stream,
                            channel: self.channel,
                            pid: self.pid,
                            errors };

            self.notif.talk(&msg);

            gst_sys::gst_buffer_unmap(ebuf.as_mut_ptr(), self.mmap);
        }
    }

    pub fn send_lost (&self) {
        let msg = MsgStatus { stream: &self.stream,
                              channel: self.channel,
                              pid: self.pid,
                              playing: false };
        self.notif_status.talk(&msg);
    }

    pub fn send_found (&self) {
        let msg = MsgStatus { stream: &self.stream,
                              channel: self.channel,
                              pid: self.pid,
                              playing: true };
        self.notif_status.talk(&msg);
    }

}

impl Drop for VideoData {
    fn drop(&mut self) {
        unsafe {
            libc::free(self.mmap as *mut libc::c_void);
        }
    }
}
