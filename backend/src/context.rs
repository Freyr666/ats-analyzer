//use chatterer::ChattererProxy;
//use chatterer::Logger;
//use settings::Configuration;
use probe::Probe;
//use control::Control;
use streams::StreamParser;
use metadata::Structure;
use wm::template::WmTemplatePartial;
use graph::Graph;
use gst;
use glib;
use std::sync::{Arc,Mutex};

pub struct ContextState {
    stream_parser: StreamParser,
    graph:         Graph,
    
}

pub struct Context {
    pub state:       Arc<Mutex<ContextState>>,
    //control:     Control,
    mainloop:    glib::MainLoop,
}

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
    pub fn new<Fstr> (uris : &Vec<(String,String)>,
                streams_cb: Fstr)
                -> Result<Box<Context>, String>
    where Fstr: Fn(&Vec<u8>) + Send + Sync + 'static {
        
        gst::init().unwrap();

        let mainloop = glib::MainLoop::new(None, false);
        //let control  = Control::new().unwrap();
        
        let mut probes  = Vec::new();

        for sid in 0..uris.len() {
            probes.push(Probe::new(&uris[sid]));
        };

        //let     config        = Configuration::new(i.msg_type, control.sender.clone());
        let mut stream_parser = StreamParser::new(/*control.sender.clone()*/);
        let     graph         = Graph::new().unwrap();
        
        for probe in &mut probes {
            probe.set_state(gst::State::Playing);
            stream_parser.connect_probe(probe);
        }

        stream_parser.connect_streams_changed (streams_cb);
        
        //let wm = graph.get_wm();

        /*
        let dis = dispatcher.clone();
        control.connect(move |s| {
            // debug!("Control message received");
            dis.lock().unwrap().dispatch(s).unwrap()
            // debug!("Control message response ready");
        });
        */

        //graph.connect_destructive(&mut stream_parser.update.lock().unwrap());
        //graph.connect_settings(&mut config.update.lock().unwrap());

        let state = Arc::new (Mutex::new (ContextState { stream_parser, graph }));
        
        info!("Context was created");
        Ok(Box::new(Context { state, mainloop }))
    }

    pub fn run (&mut self) {
    //    let context_state = self.state.clone();
/*
        self.control.connect(move |s| {
            context_state.lock().unwrap().dispatch(&s)
        });
  */      
        self.mainloop.run();
    }

    pub fn quit (&mut self) {
        self.mainloop.quit()
    }
    
}
