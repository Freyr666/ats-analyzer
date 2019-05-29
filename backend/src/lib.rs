#![allow(unknown_lints)]
#![allow(new_without_default_derive)]
#![allow(new_without_default)]
#![allow(type_complexity)]
#![allow(borrowed_box)]
#![allow(single_match)] // TODO remove
#![allow(map_entry)]
#![allow(too_many_arguments)]
#![warn(cast_ptr_alignment)]

extern crate serde;
extern crate serde_json;

#[macro_use]
extern crate log;

#[macro_use]
extern crate serde_derive;
extern crate libc;
extern crate glib;
extern crate glib_sys;
extern crate gobject_sys;
extern crate gstreamer as gst;
extern crate gstreamer_sys as gst_sys;
extern crate gstreamer_video as gst_video;
extern crate gstreamer_mpegts_sys as gst_mpegts_sys;

pub mod signals;
pub mod channels;
pub mod initial;
pub mod settings;
pub mod metadata;
pub mod parse;
pub mod probe;
pub mod streams;
pub mod pad;
pub mod audio_mux;
pub mod branch;
pub mod root;
pub mod wm;
pub mod renderer;
pub mod graph;
pub mod context;

#[cfg(feature = "std")]
use log::set_boxed_logger;
use log::{LevelFilter, Log, Record, Metadata};

use std::ffi::CString;
use std::os::raw::c_char;

#[cfg(not(feature = "std"))]
fn set_boxed_logger(logger: Box<Log>) -> Result<(), log::SetLoggerError> {
    log::set_logger(unsafe { &*Box::into_raw(logger) })
}

struct Logger { name: String }

impl log::Log for Logger {
    // TODO remove metadata
    fn enabled(&self, _metadata: &Metadata) -> bool {
        // metadata.level() <= Level::Info
        true
    }

    fn log(&self, record: &Record) {
        if self.enabled(record.metadata()) {
            println!("{}: {} - {}", self.name, record.level(), record.args());
        }
    }

    fn flush(&self) {}
}

unsafe fn string_to_chars (s: &[u8]) -> *mut c_char {
    let size = s.len();    
    //let cstr = CString::new(res).unwrap();
    let buf = libc::calloc(size + 1, 1);
    libc::memcpy(buf, s.as_ptr() as *const libc::c_void, size);
    buf as *mut c_char
}

unsafe fn chars_to_slice<'a> (cs: *const c_char) -> &'a [u8] {
    let data = std::ffi::CStr::from_ptr (cs);
    data.to_bytes()
}

unsafe fn chars_to_string (cs: *const c_char) -> String {
    let data = std::ffi::CStr::from_ptr (cs);
    String::from(data.to_str().unwrap()) //TODO remove unwrap
} 

#[no_mangle]
pub extern "C" fn qoe_backend_init_logger () {
    let log_level_filter = match std::env::var("ATS3_LOG_LEVEL") {
        Err (_) => LevelFilter::Off,
        Ok(ref s) => match s.as_str() {
            "error" => LevelFilter::Error,
            "warn"  => LevelFilter::Warn,
            "info"  => LevelFilter::Info,
            "debug" => LevelFilter::Debug,
            _ => LevelFilter::Off,
        },
    };
    
    set_boxed_logger(Box::new(Logger{ name: String::from("ats3-backend") })).unwrap();
    log::set_max_level(log_level_filter);
}

#[repr(C)]
pub struct init_val {
    tag: *const c_char,
    arg1: *const c_char,
}

#[repr(C)]
pub struct callback {
    cb: extern "C" fn(*mut c_char),
    reg_thread: extern "C" fn(),
    unreg_thread: extern "C" fn(),
}

#[repr(C)]
pub struct data_callback {
    cb: extern "C" fn(i32, *mut c_char, u32, u32, *mut std::ffi::c_void),
    reg_thread: extern "C" fn(),
    unreg_thread: extern "C" fn(),
}

#[repr(C)]
pub struct status_callback {
    cb: extern "C" fn(*mut c_char, u32, u32, bool),
    reg_thread: extern "C" fn(),
    unreg_thread: extern "C" fn(),
}

#[no_mangle]
pub unsafe extern "C" fn qoe_backend_create (vals: *const init_val,
                                             vals_num: u32,
                                             streams_cb: callback,
                                             graph_cb: callback,
                                             wm_cb: callback,
                                             data_cb: data_callback,
                                             status_cb: status_callback,
                                             err: *mut *const c_char)
                                             -> *const context::Context {

    let slice : &[init_val] = std::slice::from_raw_parts (vals, vals_num as usize);
    let mut vec : Vec<(String,String)> = Vec::with_capacity (vals_num as usize);
    
    for ref i in slice {
        let v = (chars_to_string (i.tag),
                 chars_to_string (i.arg1));
        vec.push(v);
    }

    let proc = streams_cb.cb;
    let reg  = streams_cb.reg_thread;
    let unreg = streams_cb.unreg_thread;
    
    let streams_cb : channels::Callbacks<Vec<u8>> = channels::Callbacks {
        process: Box::new(move |data| {proc(string_to_chars(data));}),
        thread_reg: Box::new(move || {reg();}),
        thread_unreg: Box::new(move || {unreg();}),
    };

    let proc = graph_cb.cb;
    let reg  = graph_cb.reg_thread;
    let unreg = graph_cb.unreg_thread;
    
    let graph_cb : channels::Callbacks<Vec<u8>> = channels::Callbacks {
        process: Box::new(move |data| {proc(string_to_chars(data));}),
        thread_reg: Box::new(move || {reg();}),
        thread_unreg: Box::new(move || {unreg();}),
    };

    let proc = wm_cb.cb;
    let reg  = wm_cb.reg_thread;
    let unreg = wm_cb.unreg_thread;
    
    let wm_cb : channels::Callbacks<Vec<u8>> = channels::Callbacks {
        process: Box::new(move |data| {proc(string_to_chars(data));}),
        thread_reg: Box::new(move || {reg();}),
        thread_unreg: Box::new(move || {unreg();}),
    };

    let proc = data_cb.cb;
    let reg  = data_cb.reg_thread;
    let unreg = data_cb.unreg_thread;
    
    let data_cb : channels::Callbacks<(branch::Typ,String,u32,u32,gst::Buffer)>
        = channels::Callbacks {
            process: Box::new(move |(t,s,c,p,d)| {proc(*t as i32,
                                                       string_to_chars(s.as_bytes()),
                                                       *c,
                                                       *p,
                                                       d.clone().into_ptr() as *mut std::ffi::c_void);}),
            thread_reg: Box::new(move || {reg();}),
            thread_unreg: Box::new(move || {unreg();}),
    };

    let proc = status_cb.cb;
    let reg  = status_cb.reg_thread;
    let unreg = status_cb.unreg_thread;
    
    let status_cb : channels::Callbacks<(String,u32,u32,bool)>
        = channels::Callbacks {
            process: Box::new(move |(s,c,p,d)| {proc(string_to_chars(s.as_bytes()),
                                                     *c,
                                                     *p,
                                                     *d)}),
            thread_reg: Box::new(move || {reg();}),
            thread_unreg: Box::new(move || {unreg();}),
    };
    
    
    let context = initial::validate(&vec)
        .and_then (|v| context::Context::new (&v, streams_cb, graph_cb, wm_cb,
                                              data_cb, status_cb));
    
    match context {
        Ok (v) => {
            Box::into_raw(v)
        },
        Err (e) => {
            *err = string_to_chars (e.as_bytes());
            std::ptr::null () as *const context::Context
        }
    }
}

#[no_mangle]
pub unsafe extern "C" fn qoe_backend_run (c: *mut context::Context) {
    if c.is_null() {
        return ();
    }
    let mut cont = Box::from_raw(c);
    cont.run ();
    std::mem::forget(cont);
}

#[no_mangle]
pub unsafe extern "C" fn qoe_backend_free (c: *mut context::Context) {
    if c.is_null() {
        return ();
    }
    let mut cont = Box::from_raw(c);
    cont.quit ();
    std::mem::drop(cont);
}
/*
#[no_mangle]
pub unsafe extern "C" fn qoe_backend_free (c: *mut context::Context) {
    let _cont = Box::from_raw(c);
    //std::mem::drop(cont);
}
*/
// TODO check allocations
#[no_mangle]
pub unsafe extern "C" fn qoe_backend_stream_parser_get_structure (c: *mut context::Context)
                                                                  -> *const c_char {
    if c.is_null() {
        return std::ptr::null() as *const c_char;
    }
    let cont = Box::from_raw (c);
    let res = cont.state.lock ().unwrap ()
        .stream_parser_get_structure ();
    std::mem::forget (cont);
    string_to_chars (&res)
}

#[no_mangle]
pub unsafe extern "C" fn qoe_backend_graph_get_structure (c: *mut context::Context)
                                                          -> *const c_char {
    if c.is_null() {
        return std::ptr::null() as *const c_char;
    }
    let cont = Box::from_raw (c);
    let res = cont.state.lock ()
        .unwrap ()
        .graph_get_structure ();
    std::mem::forget (cont);
    string_to_chars (&res)
}

#[no_mangle]
pub unsafe extern "C" fn qoe_backend_graph_apply_structure (c: *mut context::Context,
                                                            data: *const c_char,
                                                            err: *mut *const c_char)
                                                            -> i32 {
    if c.is_null() {
        *err = string_to_chars ("no context provided".as_bytes());
        return -1;
    }
    let cont = Box::from_raw (c);
    let data = chars_to_slice (data);
    let res = cont.state.lock ()
        .unwrap ()
        .graph_apply_structure (&data);
    std::mem::forget (cont);
    match res {
        Ok (()) => 0,
        Err (e) => {
            *err = string_to_chars (e.as_bytes());
            -1
        },
    }
}

#[no_mangle]
pub unsafe extern "C" fn qoe_backend_graph_get_settings (c: *mut context::Context,
                                                         err: *mut *const c_char)
                                                         -> *const c_char {
    if c.is_null() {
        *err = string_to_chars ("no context provided".as_bytes());
        return std::ptr::null() as *const c_char;
    }
    let cont = Box::from_raw (c);
    let res = cont.state.lock ()
        .unwrap ()
        .graph_get_settings ();
    std::mem::forget (cont);
    match res {
        Ok (v) => string_to_chars (&v),
        Err(e) => {
            *err = string_to_chars (e.as_bytes());
            std::ptr::null () as *const c_char
        }
    }
}

#[no_mangle]
pub unsafe extern "C" fn qoe_backend_graph_apply_settings (c: *mut context::Context,
                                                           data: *const c_char,
                                                           err: *mut *const c_char)
                                                           -> i32 {
    if c.is_null() {
        *err = string_to_chars ("no context provided".as_bytes());
        return -1;
    }
    let cont = Box::from_raw (c);
    let data = chars_to_slice (data);
    let res = cont.state.lock ()
        .unwrap ()
        .graph_apply_settings (&data);
    std::mem::forget (cont);
    match res {
        Ok (()) => 0,
        Err (e) => {
            *err = string_to_chars (e.as_bytes());
            -1
        },
    }
}

#[no_mangle]
pub unsafe extern "C" fn qoe_backend_wm_get_layout (c: *mut context::Context,
                                                    err: *mut *const c_char)
                                                    -> *const c_char {
    if c.is_null() {
        *err = string_to_chars ("no context provided".as_bytes());
        return std::ptr::null() as *const c_char;
    }
    let cont = Box::from_raw (c);
    let res = cont.state.lock ()
        .unwrap ()
        .wm_get_layout ();
    std::mem::forget (cont);
    match res {
        Ok (v) => string_to_chars (&v),
        Err(e) => {
            *err = string_to_chars (e.as_bytes());
            std::ptr::null () as *const c_char
        }
    }
}

#[no_mangle]
pub unsafe extern "C" fn qoe_backend_wm_apply_layout (c: *mut context::Context,
                                                      data: *const c_char,
                                                      err: *mut *const c_char)
                                                      -> i32 {
    if c.is_null() {
        *err = string_to_chars ("no context provided".as_bytes());
        return -1;
    }
    let cont = Box::from_raw (c);
    let data = chars_to_slice (data);
    let res = cont.state.lock ()
        .unwrap ()
        .wm_apply_layout (&data);
    std::mem::forget (cont);
    match res {
        Ok (()) => 0,
        Err (e) => {
            *err = string_to_chars (e.as_bytes());
            -1
        },
    }
}
