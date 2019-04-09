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
//extern crate rmp_serde as serde_msgpack;
extern crate zmq;

pub mod signals;
pub mod initial;
pub mod settings;
pub mod metadata;
pub mod parse;
pub mod probe;
pub mod streams;
pub mod pad;
pub mod audio_mux;
pub mod video_data;
pub mod audio_data;
pub mod branch;
pub mod root;
pub mod wm;
pub mod renderer;
pub mod graph;
pub mod control;
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

#[no_mangle]
pub extern "C" fn qoe_backend_create (err: *mut *const c_char) -> *const context::Context {
    let initial = initial::Initial { uris: vec![(String::from("test"),
                                                 String::from("udp://224.1.2.2:1234"))] };
    match context::Context::new (&initial) {
        Ok (v) => {
            println!("Good context");
            Box::into_raw(v)
        },
        Err (_e) => {
         /*   unsafe { *err = CString::new (e)
                      .expect("CString::new failed")
                      .into_raw();
            }*/
            std::ptr::null () as *const context::Context
        }
    }
}

#[no_mangle]
pub unsafe extern "C" fn qoe_backend_run (c: *mut context::Context) {
    let mut cont = Box::from_raw(c);
    cont.run ();
    std::mem::forget(cont);
}

#[no_mangle]
pub unsafe extern "C" fn qoe_backend_quit (c: *mut context::Context) {
    let mut cont = Box::from_raw(c);
    cont.quit ();
    std::mem::forget(cont);
}

#[no_mangle]
pub unsafe extern "C" fn qoe_backend_free (c: *mut context::Context) {
    let _cont = Box::from_raw(c);
    //std::mem::drop(cont);
}

#[no_mangle]
pub unsafe extern "C" fn qoe_backend_get (c: *mut context::Context)
                                          -> *mut c_char {
    let cont = Box::from_raw(c);
    println!("Trying to get streams");
    let res = cont.state.lock().unwrap()
        .stream_parser_get_structure();
    println!("Got streams");
    std::mem::forget (cont);
    let cstr = CString::new(res).unwrap();
    let size = std::mem::size_of_val(&cstr);
    let buf = libc::malloc(size);
    println!("Size of buffer: {}", size);
    libc::memcpy(buf, cstr.as_ptr() as *const libc::c_void, size);
    buf as *mut c_char
}
