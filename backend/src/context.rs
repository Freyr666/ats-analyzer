//use chatterer::ChattererProxy;
//use chatterer::Logger;
use initial::Initial;
use settings::Configuration;
use probe::Probe;
use control::Control;
use streams::{Streams};
use graph::Graph;
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
    config:      Configuration,
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
        
        let mut probes  = Vec::new();

        for sid in 0..i.uris.len() {
            probes.push(Probe::new(sid as i32,&i.uris[sid]));
        };

        let mut config  = Configuration::new(i.msg_type, control.sender.clone());
        let mut streams = Streams::new(i.msg_type, control.sender.clone());
        let mut graph   = Graph::new(i.msg_type, control.sender.clone()).unwrap();
        let preferences = Preferences::new();
        
        for probe in &mut probes {
            probe.set_state(gst::State::Playing);
            streams.connect_probe(probe);
        }
        
        dispatcher.lock().unwrap().add_to_table(&config);
        dispatcher.lock().unwrap().add_to_table(&streams);
        dispatcher.lock().unwrap().add_to_table(&graph);
        let wm = graph.get_wm();
        dispatcher.lock().unwrap().add_to_table(&(*wm.lock().unwrap()));

        let dis = dispatcher.clone();
        control.connect(move |s| dis.lock().unwrap().dispatch(s).unwrap());

        graph.connect_destructive(&mut streams.update.lock().unwrap());
        graph.connect_settings(&mut config.update.lock().unwrap());
        
        Ok(Context { mainloop, dispatcher, probes, control,
                     config, streams, graph, preferences })
    }

    pub fn run(&self) {
        self.mainloop.run();
    }
}
