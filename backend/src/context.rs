//use chatterer::ChattererProxy;
//use chatterer::Logger;
use chatterer::Chatterer;
use initial::Initial;
use probe::Probe;
use control::Control;
use structure::Structure;
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

use chatterer::{MsgType,Addressable,Dispatcher,DispatchTable};

pub struct Context<'a> {
    mainloop:    glib::MainLoop,
    name:        String,
    format:      MsgType,
    table:       HashMap<String, (Box<Fn(&'a [u8]) -> Vec<u8> + 'a>)>,   
    probes:      Vec<Probe>,
    control:     Control,
    graph:       Graph<'a>,
    structure:   Structure,
    preferences: Preferences,
}

impl<'a> Addressable for Context<'a> {
    fn get_name (&self) -> &str { &self.name }
    fn set_format (&mut self, t: MsgType) { self.format = t  }
    fn get_format (&self) -> MsgType { self.format }
}

impl<'a> DispatchTable<'a> for Context<'a> {
    fn add_respondent (&mut self, s: String, c: Box<Fn(&'a [u8]) -> Vec<u8> + 'a>) {
        self.table.insert(s, c);
    }
    
    fn get_respondent (&'a self, s: &str) -> Option<&Box<Fn(&'a [u8]) -> Vec<u8> + 'a>> {
        self.table.get(s)
    }
}

impl<'a> Dispatcher<'a> for Context<'a> {}

impl<'a> Context<'a> {
    pub fn new(i : &Initial) -> Result<Context, String> {
        gst::init().unwrap();
        
        let mainloop    = glib::MainLoop::new(None, false);
        let name        = String::from_str("context").unwrap();
        let table       = HashMap::new();
        let format      = MsgType::Json;
        let mut control = Control::new().unwrap();
        let structure   = Structure::new();
        let graph       = Graph::new().unwrap();
        let preferences = Preferences::new();
        let mut probes  = vec![Probe::new(0, "udp://224.1.2.2:1234")];
        
        for probe in &mut probes {
            probe.set_state(gst::State::Playing);
            probe.updated.lock().unwrap().connect(|s| println!("Msg from probe: {:?}", s));
        }
        
        control.connect(|s| { println!("String: {:?}", s); Vec::from("rval")} );
        Ok(Context { mainloop, name, table,
                     format,   probes, control,
                     structure, graph, preferences })
    }

    pub fn run(&self) {
        self.mainloop.run();
    }
}
