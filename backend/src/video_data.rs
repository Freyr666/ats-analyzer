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
struct Params {
    frozen_pix: f32,
    black_pix:  f32,
    blocks:     f32,
    avg_bright: f32,
    avg_diff:   f32,
    time:       i64,
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
#[repr(C)]
struct ErrorFlags {
    cont: bool,
    peak: bool,
    time: i64,
}

#[derive(Serialize,Deserialize,Debug)]
struct Errors<'a> {
    #[serde(bound(deserialize = "&'a [ErrorFlags]: Deserialize<'de>"))]
    black:  &'a [ErrorFlags],
    luma:   &'a [ErrorFlags],
    freeze: &'a [ErrorFlags],
    diff:   &'a [ErrorFlags],
    blocky: &'a [ErrorFlags],
} 

#[derive(Serialize,Deserialize,Debug)]
struct Msg<'a> {
    stream:     u32,
    channel:    u32,
    pid:        u32,
    #[serde(bound(deserialize = "&'a [Params]: Deserialize<'de>"))]
    parameters: &'a [Params],
    #[serde(bound(deserialize = "Errors<'a>: Deserialize<'de>"))]
    errors:     Errors<'a>
}

pub struct VideoData {
    stream:     u32,
    channel:    u32,
    pid:        u32,
    notif:      Notifier,
    mmap:       *mut gst_sys::GstMapInfo,
}

const markers : [&'static str; Parameter::ParamNumber as usize] = ["","","","",""];

unsafe impl Send for VideoData {}

unsafe impl Sync for VideoData {}

impl VideoData {

    pub fn new (stream: u32, channel: u32, pid: u32, format: MsgType, sender: Sender<Vec<u8>>) -> VideoData {
        let mmap :  *mut gst_sys::GstMapInfo;
        unsafe {
            mmap = libc::malloc(mem::size_of::<gst_sys::GstMapInfo>()) as *mut gst_sys::GstMapInfo;
        }
        VideoData { stream, channel, pid, notif: Notifier::new("video_data", format, sender), mmap }
    }

    pub fn send_msg (&self, dsz: u64, dbuf: gst::Buffer, esz: u64, ebuf: gst::Buffer) {
        let parameters: &[Params];
        unsafe {
            if gst_sys::gst_buffer_map(dbuf.as_mut_ptr(), self.mmap, gst_sys::GstMapFlags::READ) == 0 {
                panic!("video_data: dbuf mmap failure");
            }
            let pointer: *const Params = (*self.mmap).data as *const Params;
            parameters = slice::from_raw_parts(pointer, (dsz as usize));
        }
        
        let mut errors = Errors {
            black:  &[],
            luma:   &[],
            freeze: &[],
            diff:   &[],
            blocky: &[]
        };
        unsafe {
            if gst_sys::gst_buffer_map(ebuf.as_mut_ptr(), self.mmap, gst_sys::GstMapFlags::READ) == 0 {
                panic!("video_data: ebuf mmap failure");
            }
            let pointer: *const ErrorFlags = (*self.mmap).data as *const ErrorFlags;
            for i in 0..(Parameter::ParamNumber as usize) {
                let p = pointer.offset((i * (esz as usize)) as isize);
                match i {
                    i if i == (Parameter::Black as usize) =>
                        errors.black = slice::from_raw_parts(p, (esz as usize)),
                    i if i == (Parameter::Blocky as usize) =>
                        errors.blocky = slice::from_raw_parts(p, (esz as usize)),
                    i if i == (Parameter::Diff as usize)
                        => errors.diff = slice::from_raw_parts(p, (esz as usize)),
                    i if i == (Parameter::Freeze as usize)
                        => errors.freeze = slice::from_raw_parts(p, (esz as usize)),
                    i if i == (Parameter::Luma as usize)
                        => errors.luma = slice::from_raw_parts(p, (esz as usize)),
                    _ => panic!("video_data: too much params")
                }
            }
        }
        let msg = Msg { stream: self.stream, channel: self.channel, pid: self.pid,
                        parameters, errors };

        self.notif.talk(&msg);
    }

}

impl Drop for VideoData {
    fn drop(&mut self) {
        unsafe {
            libc::free(self.mmap as *mut libc::c_void);
        }
    }
}
