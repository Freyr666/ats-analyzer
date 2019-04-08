use gst;
use gst_sys;
use libc;
use gst::prelude::*;
use std::slice;
use std::mem;
use std::sync::mpsc::Sender;
use serde::Deserialize;
use signals::Signal;

/* TODO check validity
 * Data representation:
 * { silence_shortt : error;
 *   silence_moment : error;
 *   loudness_short : error;
 *   loudness_moment : error;
 * }
 * where error =
 * { counter : u32;
 *   size : u32;
 *   params : params;
 *   timestamp: i64;
 *   peak_flag: bool;
 *   cont_flag: bool;
 * }
 * and params =
 * { min: f64;
 *   max: f64;
 *   avg: f64;
 * }
 */

pub struct AudioData {
    signal:        Signal<[u8]>,
    signal_status: Signal<bool>,
    mmap:          *mut gst_sys::GstMapInfo,
}

pub trait Api {}
    

unsafe impl Send for AudioData {}

// unsafe impl Sync for AudioData {}

impl AudioData {

    pub fn new () -> AudioData {
        let mmap :  *mut gst_sys::GstMapInfo;
        unsafe {
            mmap = libc::malloc(mem::size_of::<gst_sys::GstMapInfo>()) as *mut gst_sys::GstMapInfo;
        }
        let signal = Signal::new();
        let signal_status = Signal::new();
        AudioData { signal, signal_status, mmap }
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
