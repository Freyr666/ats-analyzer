//use chatterer::ChattererProxy;
//use chatterer::Logger;
use initial::Initial;
use settings::Configuration;
use probe::Probe;
use control::Control;
use streams::StreamParser;
use graph::Graph;
use std::boxed::Box;
use std::collections::HashMap;
use gst;
use glib;
use std::sync::{Arc,Mutex};
use std::sync::atomic::{AtomicBool,Ordering};
use std::{thread,time};
use serde_json;
use serde_msgpack;

use chatterer::{MsgType,Description};
use chatterer::control::{parse,Name,Method,Content,Response};
use chatterer::notif::Notifier;

#[derive(Serialize)]
enum Status { Ready }

pub struct ContextState {
    format:        MsgType,
    ready:         Arc<AtomicBool>,
    stream_parser: StreamParser,
    graph:         Graph,
    
}

pub struct Context {
    state:       Arc<Mutex<ContextState>>,
    mainloop:    glib::MainLoop,
    notif:       Notifier,
}

impl ContextState {
    fn parse<'a,T> (&self, m: &'a Vec<u8>) -> T
    where T: serde::Deserialize<'a> {
        parse (&self.format, &m)
    }

    fn respond_readyness (&self, msg: &Vec<u8>) -> Vec<u8> {
        let meth : Method = self.parse(&msg);
        match meth.method {
            "accept" => {
                if !self.ready.load(Ordering::Relaxed) {
                    self.ready.store(true, Ordering::Relaxed);
                    meth.respond_ok ("established")
                        .serialize (&self.format)
                } else {
                    meth.respond_err::<&str> ("already connected")
                        .serialize (&self.format)
                }
            },
            _ => meth.respond_err::<()> ("not found")
                     .serialize (&self.format)
        }
    }

    fn respond_stream_parser (&self, msg: &Vec<u8>) -> Vec<u8> {
        let meth : Method = self.parse(&msg);
        match meth.method {
            "get" => {
                let s : &Vec<_> = &self.stream_parser.structures.lock().unwrap();
                meth.respond_ok (&s).serialize (&self.format)
            },
            _ => meth.respond_err::<()> ("not found")
                     .serialize (&self.format)
        }
    }
    
    fn respond_graph (&self, msg: &Vec<u8>) -> Vec<u8> {
        let meth : Method = self.parse(&msg);
        Vec::new()
    }

    fn respond_wm (&self, msg: &Vec<u8>) -> Vec<u8> {
        let meth : Method = self.parse(&msg);
        Vec::new()
    }
    
    fn respond_not_found (&self, s: &str, msg: &Vec<u8>) -> Vec<u8> {
        let meth : Method = self.parse(&msg);
        meth.respond_err::<()> ("not found")
            .serialize (&self.format)
    }
    
    pub fn dispatch (&mut self, msg: &Vec<u8>) -> Vec<u8> {
        let addr : Name = self.parse(&msg);
        match addr.name {
            "connection"    => self.respond_readyness (&msg),
            "stream_parser" => self.respond_stream_parser (&msg),
            "graph"         => self.respond_graph (&msg),
            "wm"            => self.respond_wm (&msg),
            s => self.respond_not_found (&s, &msg),
        }
    }
}

impl Description for Context {
    fn describe () -> String {
        String::from("")
    }
}

impl Context {
    pub fn new (i : &Initial) -> Result<Context, String> {
        gst::init().unwrap();

        let mainloop    = glib::MainLoop::new(None, false);
        let mut control = Control::new().unwrap();

        let notif       = Notifier::new("backend", i.msg_type, control.sender.clone());
        
        let mut probes  = Vec::new();

        for sid in 0..i.uris.len() {
            probes.push(Probe::new(&i.uris[sid]));
        };

        let     config        = Configuration::new(i.msg_type, control.sender.clone());
        let mut stream_parser = StreamParser::new(i.msg_type, control.sender.clone());
        let mut graph         = Graph::new(i.msg_type, control.sender.clone()).unwrap();
        
        for probe in &mut probes {
            probe.set_state(gst::State::Playing);
            stream_parser.connect_probe(probe);
        }
        
        let wm = graph.get_wm();

        /*
        dispatcher.lock().unwrap().add_to_table(&config);
        dispatcher.lock().unwrap().add_to_table(&streams);
        dispatcher.lock().unwrap().add_to_table(&graph);
        dispatcher.lock().unwrap().add_to_table(&connection);
        dispatcher.lock().unwrap().add_to_table(&(*wm.lock().unwrap()));
         */
        /*
        let dis = dispatcher.clone();
        control.connect(move |s| {
            // debug!("Control message received");
            dis.lock().unwrap().dispatch(s).unwrap()
            // debug!("Control message response ready");
        });
        */

        //graph.connect_destructive(&mut stream_parser.update.lock().unwrap());
        graph.connect_settings(&mut config.update.lock().unwrap());

        let ready = Arc::new(AtomicBool::new(false));

        let state = Arc::new (Mutex::new (ContextState { format: i.msg_type, ready, stream_parser, graph }));
        
        info!("Context was created");
        Ok(Context { state, mainloop, notif })
    }

    pub fn run (&self) {
        let ready = self.state.lock().unwrap().ready.clone();
        while !ready.load(Ordering::Relaxed) {
            self.notif.talk(&Status::Ready);
            thread::sleep(time::Duration::from_millis(100));
        }
        self.mainloop.run();
    }
}
