use std::sync::{Arc,Mutex};
use gst::prelude::*;
use gst;
use pad::SrcPad;
use signals::Signal;

pub struct Branch {
    stream: u32,
    channel: u32,
    pid: u32,
    pads: Vec<SrcPad>,
    analyser: gst::Element,
    decoder: gst::Element,
    bin: gst::Bin,
    pub pad_added: Arc<Mutex<Signal<SrcPad>>>,
}

impl Branch {
    
}
