use gst;
use gst_sys;
use libc;
use gst::prelude::*;
use std::slice;
use std::mem;
use std::sync::mpsc::Sender;
use std::sync::Arc;
use serde::Deserialize;
use signals::Signal;

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

pub struct VideoData {
    stream:        Arc<String>,
    channel:       u32,
    pid:           u32,
    signal:        Signal<[u8]>,
    signal_status: Signal<bool>,
    mmap:          *mut gst_sys::GstMapInfo,
}

unsafe impl Send for VideoData {}

//unsafe impl Sync for VideoData {}

impl VideoData {

    pub fn new (stream: String, channel: u32, pid: u32) -> VideoData {
        let mmap :  *mut gst_sys::GstMapInfo;
        unsafe {
            mmap = libc::malloc(mem::size_of::<gst_sys::GstMapInfo>()) as *mut gst_sys::GstMapInfo;
        }
        let signal = Signal::new();
        let signal_status = Signal::new();
        VideoData { stream: Arc::new(stream), channel, pid, signal, signal_status, mmap }
    }

    pub fn send_msg (&self, ebuf: &gst::Buffer) {
        unsafe {
            //gst_sys::GstMapFlags::READ
            if gst_sys::gst_buffer_map(ebuf.as_mut_ptr(), self.mmap, 1) == 0 {
                panic!("video_data: ebuf mmap failure");
            }
            let data = slice::from_raw_parts((*self.mmap).data,
                                             (*self.mmap).size);
            
            self.signal.emit(data);

            gst_sys::gst_buffer_unmap(ebuf.as_mut_ptr(), self.mmap);
        }
    }

    pub fn send_lost (&self) {
        self.signal_status.emit(&false);
    }

    pub fn send_found (&self) {
        self.signal_status.emit(&true);
    }

}

impl Drop for VideoData {
    fn drop(&mut self) {
        unsafe {
            libc::free(self.mmap as *mut libc::c_void);
        }
    }
}
