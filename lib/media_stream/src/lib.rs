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
extern crate serde_derive;

mod structure;
pub use structure::*;
