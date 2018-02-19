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
    shortt: f64,
    moment: f64,
    time:   i64,
}

#[repr(C)]
enum Parameter {
    Silence,
    Loudness,
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
    silence:  &'a [ErrorFlags],
    loudness: &'a [ErrorFlags],
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

    pub fn send_msg (&self, dsz: u64, dbuf: gst::Buffer, esz: u64, ebuf: gst::Buffer) {
        let parameters: &[Params];
        unsafe {
            if gst_sys::gst_buffer_map(dbuf.as_mut_ptr(), self.mmap, gst_sys::GstMapFlags::READ) == 0 {
                panic!("audio_data: dbuf mmap failure");
            }
            let pointer: *const Params = (*self.mmap).data as *const Params;
            parameters = slice::from_raw_parts(pointer, (dsz as usize));
        }
        
        let mut errors = Errors {
            silence:  &[],
            loudness: &[],
        };
        unsafe {
            if gst_sys::gst_buffer_map(ebuf.as_mut_ptr(), self.mmap, gst_sys::GstMapFlags::READ) == 0 {
                panic!("audio_data: ebuf mmap failure");
            }
            let pointer: *const ErrorFlags = (*self.mmap).data as *const ErrorFlags;
            for i in 0..(Parameter::ParamNumber as usize) {
                let p = pointer.offset((i * (esz as usize)) as isize);
                match i {
                    i if i == (Parameter::Silence as usize) =>
                        errors.silence = slice::from_raw_parts(p, (esz as usize)),
                    i if i == (Parameter::Loudness as usize) =>
                        errors.loudness = slice::from_raw_parts(p, (esz as usize)),
                    _ => panic!("audio_data: too much params")
                }
            }
        }
        let msg = Msg { stream: self.stream, channel: self.channel, pid: self.pid,
                        parameters, errors };

        self.notif.talk(&msg);
    }

}

impl Drop for AudioData {
    fn drop(&mut self) {
        unsafe {
            libc::free(self.mmap as *mut libc::c_void);
        }
    }
}
