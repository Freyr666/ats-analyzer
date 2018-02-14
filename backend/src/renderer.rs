use gst;
use gst::prelude::*;
use std::marker::PhantomData;
use pad::SrcPad;

pub struct VideoR {}
pub struct AudioR {}

pub struct Renderer<T> {
    port:    i32,
    encoder: gst::Element,
    pay:     gst::Element,
    output:  gst::Element,

    p:       PhantomData<T>,
}

impl Renderer<VideoR> {
    pub fn new (port: i32, bin: gst::Bin) -> Renderer<VideoR> {
        let encoder =
            gst::ElementFactory::make("vaapivp8enc", None)
            .unwrap_or(gst::ElementFactory::make("vp8enc", None).unwrap());
        let pay     = gst::ElementFactory::make("rtpvp8pay", None).unwrap();
        let output  = gst::ElementFactory::make("udpsink", None).unwrap();
        bin.add_many(&[&encoder,&pay,&output]).unwrap();
        gst::Element::link_many(&[&encoder,&pay,&output]).unwrap();
        encoder.sync_state_with_parent().unwrap();
        pay.sync_state_with_parent().unwrap();
        output.sync_state_with_parent().unwrap();
        encoder.set_property("rate-control", &2); // may not exist
        output.set_property("host", &"127.0.0.1").unwrap();
        output.set_property("port", &port).unwrap();
        output.set_property("async", &false).unwrap();
        output.set_property("sync", &false).unwrap();
        Renderer::<VideoR> { port, encoder, pay, output, p: PhantomData }
    }

    pub fn plug (&self, pad: gst::Pad) {
        pad.link(&self.encoder.get_static_pad("sink").unwrap());
    }
}

impl Renderer<AudioR> {
    pub fn new (port: i32, bin: gst::Bin) -> Renderer<AudioR> {
        let encoder = gst::ElementFactory::make("opusenc", None).unwrap();
        let pay     = gst::ElementFactory::make("rtpopuspay", None).unwrap();
        let output  = gst::ElementFactory::make("udpsink", None).unwrap();
        bin.add_many(&[&encoder,&pay,&output]).unwrap();
        gst::Element::link_many(&[&encoder,&pay,&output]).unwrap();
        encoder.sync_state_with_parent().unwrap();
        pay.sync_state_with_parent().unwrap();
        output.sync_state_with_parent().unwrap();
        output.set_property("host", &"127.0.0.1").unwrap();
        output.set_property("port", &port).unwrap();
        Renderer::<AudioR> { port, encoder, pay, output, p: PhantomData }
    }

    pub fn plug (&self, p: &SrcPad) {
        p.pad.link(&self.encoder.get_static_pad("sink").unwrap());
    }
}
