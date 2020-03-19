#![allow(unknown_lints)]
#![allow(new_without_default_derive)]
#![allow(new_without_default)]
#![allow(type_complexity)]
#![allow(borrowed_box)]
#![allow(single_match)] // TODO remove
#![allow(map_entry)]
#![allow(too_many_arguments)]
#![warn(cast_ptr_alignment)]

//extern crate serde;
extern crate serde_json;

extern crate util;
extern crate media_stream;

extern crate libc;
extern crate glib;
extern crate glib_sys;
extern crate gobject_sys;
extern crate gstreamer as gst;
extern crate gstreamer_sys as gst_sys;
extern crate gstreamer_video as gst_video;
extern crate gstreamer_mpegts_sys as gst_mpegts_sys;

pub mod parse;
pub mod probe;

use std::ffi::CString;
use std::os::raw::{c_char,c_void};

unsafe fn string_to_chars (s: &[u8]) -> *mut c_char {
    let size = s.len();    
    //let cstr = CString::new(res).unwrap();
    let buf = libc::calloc(size + 1, 1);
    libc::memcpy(buf, s.as_ptr() as *const libc::c_void, size);
    buf as *mut c_char
}

unsafe fn chars_to_string (cs: *const c_char) -> String {
    let data = std::ffi::CStr::from_ptr (cs);
    String::from(data.to_str().unwrap()) //TODO remove unwrap
} 

#[repr(C)]
pub struct init_val {
    tag: *const c_char,
    arg1: *const c_char,
}

#[repr(C)]
pub struct callback {
    id: i32,
    cb: extern "C" fn(i32, *mut c_char),
    reg_thread: extern "C" fn(),
    unreg_thread: extern "C" fn(),
}

#[no_mangle]
pub unsafe extern "C" fn qoe_probe_create (val: *const init_val,
                                           streams_cb: callback,
                                           err: *mut *const c_char)
                                           -> *const probe::Probe {

    if val.is_null () {
        *err = string_to_chars ("Null argument".as_bytes());
        return std::ptr::null () as *const probe::Probe
    }
    
    let v = (chars_to_string ((*val).tag),
             chars_to_string ((*val).arg1));

    let id  = streams_cb.id.clone();
    let proc = streams_cb.cb;
    let reg  = streams_cb.reg_thread;
    let unreg = streams_cb.unreg_thread;
    
    let streams_cb : util::channels::Callbacks<Vec<u8>> = util::channels::Callbacks {
        process: Box::new(move |data| {proc(id, string_to_chars(data));}),
        thread_reg: Box::new(move || {reg();}),
        thread_unreg: Box::new(move || {unreg();}),
    };

    let probe = util::initial::validate(&v)
        .and_then (|v| Ok (probe::Probe::new(&v, streams_cb)));

    match probe {
        Ok (v) => Box::into_raw(v),
        Err (e) => {
            *err = string_to_chars (e.as_bytes());
            std::ptr::null () as *const probe::Probe
        }
    }
}

#[no_mangle]
pub unsafe extern "C" fn qoe_probe_run (c: *mut probe::Probe) {
    if c.is_null () {
        return ();
    }
    let mut cont = Box::from_raw(c);
    cont.run ();
    std::mem::forget(cont);
}

#[no_mangle]
pub unsafe extern "C" fn qoe_probe_free (c: *mut probe::Probe) {
    if c.is_null () {
        return ();
    }
    let mut cont = Box::from_raw(c);
    cont.quit ();
    std::mem::drop(cont);
}

// TODO check allocations
#[no_mangle]
pub unsafe extern "C" fn qoe_probe_get_structure (c: *mut probe::Probe)
                                                  -> *const c_char {
    if c.is_null() {
        return std::ptr::null() as *const c_char;
    }
    let cont = Box::from_raw (c);
    let res = cont.get_structure ();
    std::mem::forget (cont);
    string_to_chars (&res)
}
