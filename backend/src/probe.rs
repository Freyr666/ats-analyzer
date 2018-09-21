use std::sync::{Arc,Mutex};
use signals::Signal;
use metadata::Structure;
use std::str::FromStr;
use gst;
use gst::prelude::*;
use glib;
//use gst::DebugGraphDetails;
use gst_mpegts_sys;
use parse;

pub struct Probe {
    pub stream:   i32,
    pub updated:  Arc<Mutex<Signal<Structure>>>,

    pipeline:     gst::Pipeline,
}

impl Probe {
    pub fn new (stream: i32, uri: &str) -> Probe {        
        let updated       = Arc::new(Mutex::new(Signal::new()));
        let mut structure = Structure::new(String::from_str(uri).unwrap(), stream);
        
        let src   = gst::ElementFactory::make("udpsrc", None).unwrap();
        let parse = gst::ElementFactory::make("tsparse", None).unwrap();
        let sink  = gst::ElementFactory::make("fakesink", None).unwrap();

        let pipeline = gst::Pipeline::new(None);

        src.set_property("uri", &uri).unwrap();
        src.set_property("timeout", &5_000_000_000u64).unwrap();
        
        pipeline.add_many(&[&src, &parse, &sink]).unwrap();

        gst::Element::link_many(&[&src, &parse, &sink]).unwrap();
        
        let bus = pipeline.get_bus().unwrap();

        //let metadata  = structure.clone();
        let signal    = updated.clone();
        let pipe      = pipeline.clone();
        bus.add_watch(move |_, msg| {
            use gst::MessageView;
            
            match msg.view() {
                MessageView::Element(_) =>
                    if msg.get_src().unwrap().get_name().starts_with("mpegtsparse") {
                        unsafe {
                            let section = gst_mpegts_sys::gst_message_parse_mpegts_section(msg.as_mut_ptr());
                            if let Some(s) = parse::table(section, &mut structure) {
                                signal.lock().unwrap().emit(&s)
                            };
                        }
                    } else if let Some(s) = msg.get_structure() {
                            if s.get_name() == "GstUDPSrcTimeout" &&
                                ! structure.is_empty() {
                                    structure.clear();
                                    let _ = pipe.set_state(gst::State::Null);
                                    let _ = pipe.set_state(gst::State::Playing);
                                    signal.lock().unwrap().emit(&structure)
                                }
                    },
                _ => ()
            };
            glib::Continue(true)
        });
        
        Probe { stream, updated, pipeline }
    }

    pub fn set_state (&mut self, st: gst::State) {
        let _ = self.pipeline.set_state(st);
        //gst::debug_bin_to_dot_file(&self.pipeline, gst::DebugGraphDetails::VERBOSE, "pipeline");
    }
}
