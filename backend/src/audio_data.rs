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
    min: f64,
    max: f64,
    avg: f64,
}

#[repr(C)]
enum Parameter {
    SilenceShortt,
    SilenceMoment,
    LoudnessShortt,
    LoudnessMoment,
    ParamNumber,
}

#[derive(Serialize,Deserialize,Debug)]
#[repr(C)]
struct Error {
    counter:   i32,
    size:      i32,
    params:    Param,
    timestamp: i64,
    peak_flag: bool,
    cont_flag: bool,
}

#[derive(Serialize,Deserialize,Debug)]
struct Errors<'a> {
    #[serde(bound(deserialize = "&'a Error: Deserialize<'de>"))]
    silence_shortt:  &'a Error,
    silence_moment:  &'a Error,
    loudness_shortt: &'a Error,
    loudness_moment: &'a Error,
} 

#[derive(Serialize,Deserialize,Debug)]
struct Msg<'a> {
    stream:     u32,
    channel:    u32,
    pid:        u32,
    #[serde(bound(deserialize = "Errors<'a>: Deserialize<'de>"))]
    errors:     Errors<'a>
}

pub struct AudioData {
    stream:     u32,
    channel:    u32,
    pid:        u32,
    notif:      Notifier,
    mmap:       *mut gst_sys::GstMapInfo,
}

unsafe impl Send for AudioData {}

// unsafe impl Sync for AudioData {}

impl AudioData {

    pub fn new (stream: u32, channel: u32, pid: u32, format: MsgType, sender: Sender<Vec<u8>>) -> AudioData {
        let mmap :  *mut gst_sys::GstMapInfo;
        unsafe {
            mmap = libc::malloc(mem::size_of::<gst_sys::GstMapInfo>()) as *mut gst_sys::GstMapInfo;
        }
        AudioData { stream, channel, pid, notif: Notifier::new("audio_data", format, sender), mmap }
    }

    pub fn send_msg (&self, buf: &gst::Buffer) {
        unsafe {
            if gst_sys::gst_buffer_map(buf.as_mut_ptr(), self.mmap, gst_sys::GstMapFlags::READ) == 0 {
                panic!("audio_data: buf mmap failure");
            }
            let pointer: *const Error = (*self.mmap).data as *const Error;
            let err_buf = slice::from_raw_parts(pointer, Parameter::ParamNumber as usize);
            let errors = Errors {
                silence_shortt:  &err_buf[Parameter::SilenceShortt as usize],
                silence_moment:  &err_buf[Parameter::SilenceMoment as usize],
                loudness_shortt: &err_buf[Parameter::LoudnessShortt as usize],
                loudness_moment: &err_buf[Parameter::LoudnessMoment as usize],
            };
            let msg = Msg { stream: self.stream,
                            channel: self.channel,
                            pid: self.pid,
                            errors };

            self.notif.talk(&msg);
        }
    }

}

impl Drop for AudioData {
    fn drop(&mut self) {
        unsafe {
            libc::free(self.mmap as *mut libc::c_void);
        }
    }
}
