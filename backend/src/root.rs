use gst::prelude::*;
use gst;
use branch::Branch;

pub struct Root {
    bin:      gst::Bin,
    src:      gst::Element,
    tee:      gst::Element,
    demux:    gst::Element,
    branches: Vec<Branch>,
}

