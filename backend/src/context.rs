//use chatterer::ChattererProxy;
//use chatterer::Logger;
use chatterer::Chatterer;
use initial::Initial;
use probe::Probe;
use control::Control;
use streams::{Streams,StreamsState};
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

use chatterer::{MsgType,Addressable,Dispatcher,DispatchTable};

pub struct ContextDispatcher {
    name:        String,
    format:      MsgType,
    table:       HashMap<String, (Box<Fn(Vec<u8>) -> Vec<u8> + Send + Sync>)>,
}

pub struct Context<'a> {
    dispatcher:  Arc<Mutex<ContextDispatcher>>,
    mainloop:    glib::MainLoop,  
    probes:      Vec<Probe>,
    control:     Control,
    streams:     Streams,
    graph:       Graph<'a>,
    preferences: Preferences,
}

impl Addressable for ContextDispatcher {
    fn get_name (&self) -> &str { &self.name }
    fn set_format (&mut self, t: MsgType) { self.format = t  }
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
    pub fn new () -> ContextDispatcher {
        let name        = String::from_str("context").unwrap();
        let table       = HashMap::new();
        let format      = MsgType::Json;
        ContextDispatcher { name, table, format }
    }
}

impl<'a> Context<'a> {
    pub fn new (i : &Initial) -> Result<Context<'a>, String> {
        gst::init().unwrap();

        let dispatcher  = Arc::new(Mutex::new(ContextDispatcher::new()));
        let mainloop    = glib::MainLoop::new(None, false);
        let mut control = Control::new().unwrap();
        let graph       = Graph::new().unwrap();
        let preferences = Preferences::new();
        let mut probes  = vec![Probe::new(0, "udp://224.1.2.2:1234")];
        let mut streams = Streams::new();
        
        for probe in &mut probes {
            probe.set_state(gst::State::Playing);
            streams.connect_probe(probe);
        }

        streams.connect_channel(MsgType::Json, control.sender.clone());

        dispatcher.lock().unwrap().add_to_table(&(*streams.state.lock().unwrap()));
        dispatcher.lock().unwrap().add_to_table(&graph);

        let dis = dispatcher.clone();
        control.connect(move |s| dis.lock().unwrap().dispatch(s).unwrap());
        
        Ok(Context { mainloop, dispatcher, probes, control,
                     streams, graph, preferences })
    }

    pub fn run(&self) {
        self.mainloop.run();
    }
}
