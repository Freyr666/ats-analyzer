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
pub mod chatterer;
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
pub extern fn qoe_backend_init_logger () {
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
pub extern fn qoe_backend_create (err: *mut *const c_char) -> Option<(*mut context::Context)> {
    let initial = initial::Initial { uris: Vec::new() };
    match context::Context::new (&initial) {
        Ok (v) => {
            Some (Box::into_raw(v))
        },
        Err (e) => {
            unsafe { *err = CString::new (e)
                      .expect("CString::new failed")
                      .into_raw();
            }
            None
        }
    }
}
