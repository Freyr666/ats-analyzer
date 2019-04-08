//use chatterer::ChattererProxy;
//use chatterer::Logger;
use initial::Initial;
//use settings::Configuration;
use probe::Probe;
use control::Control;
use streams::StreamParser;
use metadata::Structure;
use wm::template::WmTemplatePartial;
use graph::Graph;
use gst;
use glib;
use std::sync::{Arc,Mutex};

use chatterer::Description;
use chatterer::control::{parse,Name,Method,Content};

pub struct ContextState {
    stream_parser: StreamParser,
    graph:         Graph,
    
}

pub struct Context {
    state:       Arc<Mutex<ContextState>>,
    control:     Control,
    mainloop:    glib::MainLoop,
}

impl ContextState {

    fn respond_stream_parser (&self, msg: &Vec<u8>) -> Vec<u8> {
        let meth : Method = parse(&msg);
        match meth.method {
            "get" => {
                let s : &Vec<_> = &self.stream_parser.structures.lock().unwrap();
                meth.respond_ok (&s).serialize ()
            },
            _ => meth.respond_err::<()> ("not found")
                     .serialize ()
        }
    }
    
    fn respond_graph (&self, msg: &Vec<u8>) -> Vec<u8> {
        let meth : Method = parse(&msg);
        match meth.method {
            "get_structure" => {
                let s = self.graph.get_structure();
                meth.respond_ok (&s).serialize ()
            },
            "apply_structure" => {
                let cont : Content<Vec<Structure>> = parse (&msg);
                meth.respond (self.graph.set_structure(cont.content))
                    .serialize ()
            },
            _ => meth.respond_err::<()> ("not found")
                     .serialize ()
        }
    }

    fn respond_wm (&self, msg: &Vec<u8>) -> Vec<u8> {
        let meth : Method = parse(&msg);
        match meth.method {
            "get_layout" => {
                let templ = self.graph.get_wm_layout ();
                meth.respond (templ).serialize ()
            },
            "apply_layout" => {
                let templ : Content<WmTemplatePartial> = parse(&msg);
                let resp = self.graph.set_wm_layout (templ.content);
                meth.respond (resp).serialize ()
            },
            _ => meth.respond_err::<()> ("not found")
                     .serialize ()
        }
    }
    
    fn respond_not_found (&self, msg: &Vec<u8>) -> Vec<u8> {
        let meth : Method = parse(&msg);
        meth.respond_err::<()> ("not found")
            .serialize ()
    }
    
    pub fn dispatch (&mut self, msg: &Vec<u8>) -> Vec<u8> {
        let addr : Name = parse(&msg);
        match addr.name {
            "stream_parser" => self.respond_stream_parser (&msg),
            "graph"         => self.respond_graph (&msg),
            "wm"            => self.respond_wm (&msg),
            _ => self.respond_not_found (&msg),
        }
    }
}

impl Description for Context {
    fn describe () -> String {
        String::from("")
    }
}

impl Context {
    pub fn new (i : &Initial) -> Result<Box<Context>, String> {
        gst::init().unwrap();

        let mainloop = glib::MainLoop::new(None, false);
        let control  = Control::new().unwrap();
        
        let mut probes  = Vec::new();

        for sid in 0..i.uris.len() {
            probes.push(Probe::new(&i.uris[sid]));
        };

        //let     config        = Configuration::new(i.msg_type, control.sender.clone());
        let mut stream_parser = StreamParser::new(/*control.sender.clone()*/);
        let     graph         = Graph::new(control.sender.clone()).unwrap();
        
        for probe in &mut probes {
            probe.set_state(gst::State::Playing);
            stream_parser.connect_probe(probe);
        }
        
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
        Ok(Box::new(Context { state, control, mainloop }))
    }

    pub fn run (&mut self) {
        let context_state = self.state.clone();

        self.control.connect(move |s| {
            context_state.lock().unwrap().dispatch(&s)
        });
        
        self.mainloop.run();
    }
}
