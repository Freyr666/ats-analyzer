//use chatterer::ChattererProxy;
//use chatterer::Logger;
//use settings::Configuration;
use probe::Probe;
use channels;
use settings::SettingsFlat;
use streams::StreamParser;
use metadata::Structure;
use wm::template::WmTemplatePartial;
use graph::Graph;
use branch::Typ;
use gst;
use glib;
use std::sync::{Arc,Mutex};

pub struct ContextState {
    stream_parser: StreamParser,
    graph:         Graph,
    probes:        Vec<Probe>,
}

pub struct Context {
    pub state:   Arc<Mutex<ContextState>>,
    mainloop:    glib::MainLoop,
    num:         i32,
}

static mut C : i32 = 0;

impl ContextState {

    pub fn stream_parser_get_structure (&self) -> Vec<u8> {
        println!("Getting streams");
        let s : &Vec<_> = &self.stream_parser.structures.lock().unwrap();
        serde_json::to_vec(&s).unwrap()
    }

    pub fn graph_get_structure (&self) -> Vec<u8> {
        let s = self.graph.get_structure();
        serde_json::to_vec(&s).unwrap()
    }

    pub fn graph_apply_structure (&self, v: &[u8]) -> Result<(),String> {
        if let Ok (structs) = serde_json::from_slice::<Vec<Structure>>(&v) {
            self.graph.set_structure(structs)
        } else {
            Err (String::from("msg format err"))
        }
    }

    pub fn graph_get_settings (&self) -> Result<Vec<u8>,String> {
        self.graph.get_settings().and_then( |s| {
            match serde_json::to_vec(&s) {
                Ok(v) => Ok(v),
                Err(e) => Err(e.to_string()),
            }
        })
    }

    pub fn graph_apply_settings (&self, v: &[u8]) -> Result<(),String> {
        if let Ok (set) = serde_json::from_slice::<SettingsFlat>(&v) {
            self.graph.set_settings(set)
        } else {
            Err (String::from("msg format err"))
        }
    }

    pub fn wm_get_layout (&self) -> Result<Vec<u8>,String> {
        match self.graph.get_wm_layout () {
            Ok(s)  => Ok(serde_json::to_vec(&s).unwrap()),
            Err(e) => Err(e),
        }
    }

    pub fn wm_apply_layout (&self, v: &[u8]) -> Result<(),String> {
        if let Ok(templ) = serde_json::from_slice::<WmTemplatePartial>(&v) {
            self.graph.set_wm_layout(templ)
        } else {
            Err (String::from("msg format err"))
        }
    }
    
}

impl Context {
    pub fn new (uris : &Vec<(String,String)>,
                streams_cb: channels::Callbacks<Vec<u8>>,
                graph_cb: channels::Callbacks<Vec<u8>>,
                wm_cb: channels::Callbacks<Vec<u8>>,
                data_cb: channels::Callbacks<(Typ,String,u32,u32,gst::Buffer)>,
                status_cb: channels::Callbacks<(String,u32,u32,bool)>,
    ) -> Result<Box<Context>, String> {
        
        gst::init().unwrap();

        let num = unsafe { C };
        unsafe { C = C + 1 };

        let mainloop = glib::MainLoop::new(None, false);
        
        let mut probes  = Vec::new();

        for sid in 0..uris.len() {
            probes.push(Probe::new(&uris[sid]));
        };

        let stream_sender = channels::create (streams_cb);
        let graph_sender = channels::create (graph_cb);
        let wm_sender = channels::create (wm_cb);
        let data_sender = channels::create (data_cb);
        let status_sender = channels::create (status_cb);

        let mut stream_parser = StreamParser::new(stream_sender);
        let     graph         = Graph::new(graph_sender,
                                           wm_sender,
                                           data_sender,
                                           status_sender).unwrap();
        
        for probe in &mut probes {
            probe.set_state(gst::State::Playing);
            stream_parser.connect_probe(probe);
        }

        let state = Arc::new (Mutex::new (ContextState { stream_parser, graph, probes }));
        
        info!("Context was created");
        Ok(Box::new(Context { state, mainloop, num }))
    }

    pub fn run (&mut self) {
        self.mainloop.run();
    }
    // Should be called in separate thread since
    // Mainloop::run is blocking
    pub fn quit (&mut self) {
        self.mainloop.quit();
        warn!("Context {}: loop quit", self.num)
    }
    
}

impl Drop for Context {
    fn drop (&mut self) {
        warn!("Context {} was dropped", self.num)
    }
}
