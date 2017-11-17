extern crate backend;

use std::env::Args;
use backend::initial::Initial;
use backend::context::Context;

fn main() {
    let args = std::env::args();
    let i    = Initial::new(&args).unwrap();
    let c    = Context::new(&i).unwrap();
    println!("Hello, world!");
}
