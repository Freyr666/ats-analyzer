//use chatterer::ChattererProxy;
//use chatterer::Logger;
use initial::Initial;
use probe::Probe;
use control::Control;
use streams::{Streams};
use graph::Graph;
use graph::GraphSettings;
use preferences::Preferences;
use std::thread;
use std::str::FromStr;
use std::fmt::Display;
use std::boxed::Box;
use std::collections::HashMap;
use gst;
use gst::prelude::*;
use glib;
use std::sync::{Arc,Mutex};

use chatterer::MsgType;
use chatterer::control::{Addressable,Dispatcher,DispatchTable};

pub struct ContextDispatcher {
    format:      MsgType,
    table:       HashMap<String, (Box<Fn(Vec<u8>) -> Vec<u8> + Send + Sync>)>,
}

pub struct Context {
    dispatcher:  Arc<Mutex<ContextDispatcher>>,
    mainloop:    glib::MainLoop,  
    probes:      Vec<Probe>,
    control:     Control,
    streams:     Streams,
    graph:       Graph,
    preferences: Preferences,
}

impl Addressable for ContextDispatcher {
    fn get_name (&self) -> &str { "context" }
    fn get_format (&self) -> MsgType { self.format }
}

impl DispatchTable for ContextDispatcher {
    fn add_respondent (&mut self, s: String, c: Box<Fn(Vec<u8>) -> Vec<u8> + Send + Sync>) {
        self.table.insert(s, c);
    }
    
    fn get_respondent (&self, s: &str) -> Option<&Box<Fn(Vec<u8>) -> Vec<u8> + Send + Sync>> {
        self.table.get(s)
    }
}

impl Dispatcher for ContextDispatcher {}

impl ContextDispatcher {
    pub fn new (format: MsgType) -> ContextDispatcher {
        let table       = HashMap::new();
        ContextDispatcher { table, format }
    }
}

impl Context {
    pub fn new (i : &Initial) -> Result<Context, String> {
        gst::init().unwrap();

        let dispatcher  = Arc::new(Mutex::new(ContextDispatcher::new(MsgType::Json)));
        let mainloop    = glib::MainLoop::new(None, false);
        let mut control = Control::new().unwrap();
        
        let mut probes  = vec![Probe::new(0, "udp://224.1.2.2:1234"), Probe::new(1, "udp://224.1.2.3:1235")];

        let mut streams = Streams::new(MsgType::Json, control.sender.clone());
        let graph       = Graph::new(MsgType::Json, control.sender.clone()).unwrap();
        let preferences = Preferences::new();
        
        for probe in &mut probes {
            probe.set_state(gst::State::Playing);
            streams.connect_probe(probe);
        }

        dispatcher.lock().unwrap().add_to_table(&streams);
        dispatcher.lock().unwrap().add_to_table(&graph);

        let dis = dispatcher.clone();
        control.connect(move |s| dis.lock().unwrap().dispatch(s).unwrap());

        graph.connect_destructive(&mut streams.update.lock().unwrap());
        
        Ok(Context { mainloop, dispatcher, probes, control,
                     streams, graph, preferences })
    }

    pub fn run(&self) {
        self.mainloop.run();
    }
}
