use std::sync::{Arc,Mutex};
use std::sync::mpsc::Sender;
use util::signals::Signal;
use media_stream::Structure;
use gst;
use gst::prelude::*;
use glib;
//use gst::DebugGraphDetails;
use gst_mpegts_sys;
use gst_sys;
use parse;
use util::initial::{Id,Uri};
use util::channels;

pub struct ProbeState {
    pub stream:   String,
    pub updated:  Arc<Mutex<Signal<Structure>>>,
    pipeline:     gst::Pipeline,
}

pub struct Probe {
    pub state:   ProbeState,
    structure:   Arc<Mutex<Structure>>,
    sender:      Arc<Mutex<Sender<Vec<u8>>>>,
    mainloop:    glib::MainLoop,
}

impl ProbeState {
    pub fn new (stream : &(Id, Uri)) -> ProbeState {
        let updated       = Arc::new(Mutex::new(Signal::new()));
        let id            = stream.0.clone();
        let uri           = stream.1.clone();
        let mut structure = Structure::new(uri.clone(), id.clone());
        
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
                            gst_sys::gst_mini_object_unref(section as *mut gst_sys::GstMiniObject);
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
        
        ProbeState { stream : id, updated, pipeline }
    }

    pub fn set_state (&mut self, st: gst::State) {
        let _ = self.pipeline.set_state(st);
        //gst::debug_bin_to_dot_file(&self.pipeline, gst::DebugGraphDetails::VERBOSE, "pipeline");
    }
}

impl Probe {
    pub fn new (stream : &(String, String),
                streams_cb: util::channels::Callbacks<Vec<u8>>
    ) -> Box<Probe> {

        gst::init().unwrap();

        let mainloop = glib::MainLoop::new (None, false);

        let sender = Arc::new (Mutex::new (util::channels::create (streams_cb)));

        let mut state = ProbeState::new (stream);

        let structure = Arc::new (Mutex::new (Structure::new (stream.0.clone(), stream.1.clone())));

        let send = Arc::downgrade(&sender);
        let st = Arc::downgrade(&structure);
        state.updated
            .lock().unwrap()
            .connect (move |s| {
                match send.upgrade () {
                    None => (),
                    Some (send) => {
                        send.lock().unwrap().send(serde_json::to_vec(&s).unwrap());
                        match st.upgrade () {
                            None => (),
                            Some (st) => {
                                *(st.lock().unwrap()) = s.clone()
                            }
                        }
                    }
                }
            });

        state.set_state (gst::State::Playing);

        Box::new (Probe { state, mainloop, structure, sender })
    }

    pub fn run (&mut self) {
        self.mainloop.run();
    }
    
    // Should be called in separate thread since
    // Mainloop::run is blocking
    pub fn quit (&mut self) {
        self.mainloop.quit()
    }

    pub fn get_structure (&self) -> Vec<u8> {
        let s = &*self.structure.lock().unwrap();
        serde_json::to_vec(&s).unwrap()
    }
}
