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
extern crate rmp_serde as serde_msgpack;
extern crate zmq;

pub mod signals;
pub mod chatterer;
pub mod initial;
pub mod connection;
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
