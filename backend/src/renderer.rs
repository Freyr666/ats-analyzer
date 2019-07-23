use gst;
use gst::prelude::*;
use std::marker::PhantomData;

pub struct VideoR {}
pub struct AudioR {}

pub struct Renderer<T> {
    encoder: gst::Element,

    p:       PhantomData<T>,
}

//fn enum_to_val(cls: &str, val: i32) -> glib::Value {
//    glib::EnumClass::new(glib::Type::from_name(cls).unwrap()).unwrap().to_value(val).unwrap()
//}

impl Renderer<VideoR> {
    pub fn new (port: i32, bin: &gst::Pipeline) -> Renderer<VideoR> {
        //let mut vaapi = true;
        let encoder = // TODO proper codec initialization with fallback to dummy impl
            gst::ElementFactory::make("vaapivp9enc", None) //vaapivp8enc
            .unwrap();//_or({ vaapi = false; gst::ElementFactory::make("vp8enc", None).unwrap() });
        let pay     = gst::ElementFactory::make("rtpvp9pay", None).unwrap(); //rtpvp8pay
        let output  = gst::ElementFactory::make("udpsink", None).unwrap();
        //let queue   = gst::ElementFactory::make("queue", None).unwrap();
        bin.add_many(&[&encoder,&pay,/*&queue,*/&output]).unwrap();
        gst::Element::link_many(&[&encoder,&pay,/*&queue,*/&output]).unwrap();
        encoder.sync_state_with_parent().unwrap();
        pay.sync_state_with_parent().unwrap();
        output.sync_state_with_parent().unwrap();
        //queue.sync_state_with_parent().unwrap();
        //if vaapi {
            //println!("Set vaapi params");
            //encoder.set_property("rate-control", &enum_to_val("GstVaapiRateControlVP8", 2)).unwrap(); // may not exist
        encoder.set_property("bitrate", &8000u32).unwrap();
            //encoder.set_property("quality-level", &7u32).unwrap();
        //};
        //queue.set_property("max-size-time", &0u64).unwrap();
        //queue.set_property("max-size-buffers", &0u32).unwrap();
        //queue.set_property("max-size-bytes", &0u32).unwrap();
        output.set_property("host", &"127.0.0.1").unwrap();
        output.set_property("port", &port).unwrap();
        output.set_property("async", &false).unwrap();
        //output.set_property("sync", &false).unwrap();
        //output.set_property("blocksize", &4096000u32).unwrap();
        Renderer::<VideoR> { encoder, p: PhantomData }
    }

    pub fn plug (&self, pad: &gst::Pad) {
        // TODO check
        let _ = pad.link(&self.encoder.get_static_pad("sink").unwrap());
    }
}

impl Renderer<AudioR> {
    pub fn new (port: i32, bin: &gst::Pipeline) -> Renderer<AudioR> {
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
        output.set_property("async", &false).unwrap();
        Renderer::<AudioR> { encoder, p: PhantomData }
    }

    pub fn plug (&self, pad: &gst::Pad) {
        // TODO check
        let _ = pad.link(&self.encoder.get_static_pad("sink").unwrap());
    }
}
