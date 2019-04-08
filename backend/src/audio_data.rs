use gst;
use gst_sys;
use libc;
use gst::prelude::*;
use std::slice;
use std::mem;
use std::sync::mpsc::Sender;
use serde::Deserialize;
use signals::Signal;

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
    LoudnessShortt,
    SilenceMoment,
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
/*
struct Msg<'a> {
    stream:     &'a String,
    channel:    u32,
    pid:        u32,
    data:       &'a [u8]
}

struct MsgStatus<'a> {
    stream:     &'a String,
    channel:    u32,
    pid:        u32,
    playing:    bool,
}
*/
pub struct AudioData {
    stream:        String,
    channel :      u32,
    pid:           u32,
    signal:        Signal<[u8]>,
    signal_status: Signal<bool>,
    mmap:          *mut gst_sys::GstMapInfo,
}

pub trait Api {}
    

unsafe impl Send for AudioData {}

// unsafe impl Sync for AudioData {}

impl AudioData {

    pub fn new (stream: String, channel: u32, pid: u32) -> AudioData {
        let mmap :  *mut gst_sys::GstMapInfo;
        unsafe {
            mmap = libc::malloc(mem::size_of::<gst_sys::GstMapInfo>()) as *mut gst_sys::GstMapInfo;
        }
        let signal = Signal::new();
        let signal_status = Signal::new();
        AudioData { stream, channel, pid, signal, signal_status, mmap }
    }

    pub fn send_msg (&self, buf: &gst::Buffer) {
        unsafe {
            //gst_sys::GstMapFlags::READ
            if gst_sys::gst_buffer_map(buf.as_mut_ptr(), self.mmap, 1) == 0 {
                panic!("audio_data: buf mmap failure");
            }
            let data = slice::from_raw_parts((*self.mmap).data,
                                             (*self.mmap).size);

            self.signal.emit (&data);

            gst_sys::gst_buffer_unmap(buf.as_mut_ptr(), self.mmap);
        }
    }

    pub fn send_lost (&self) {
        self.signal_status.emit(&false);
    }

    pub fn send_found (&self) {
        self.signal_status.emit(&true);
    }
}

impl Drop for AudioData {
    fn drop(&mut self) {
        unsafe { /* TODO check if copyless manipulation is possible */
            libc::free(self.mmap as *mut libc::c_void);
        }
    }
}
