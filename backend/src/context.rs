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
use serde_json;
use serde_msgpack;
use std::str::FromStr;
use std::fmt::Display;
use std::boxed::Box;
use std::collections::HashMap;

use chatterer::{MsgType,Name,Addressable,Dispatcher,DispatchTable};

pub struct Context<'a> {
    name:        String,
    format:      MsgType,
    table:       HashMap<String, (Box<Fn(&'a [u8]) -> Vec<u8> + 'a>)>,   
    probes:      Vec<Box<Probe>>,
    control:     Control,
    graph:       Graph<'a>,
    structure:   Structure,
    preferences: Preferences,
}

impl<'a> Addressable for Context<'a> {
    fn set_name (&mut self, s: String) {
        self.name = s
    }
    fn get_name (&self) -> &str {
        &self.name
    }
    fn set_format (&mut self, t: MsgType) {
        self.format = t
    }
    fn get_format (&self) -> MsgType {
        self.format
    }
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
        let name        = String::from_str("context").unwrap();
        let table       = HashMap::new();
        let format      = MsgType::Json;
        let mut control = Control::new().unwrap();
        let structure   = Structure::new();
        let graph       = Graph::new().unwrap();
        let preferences = Preferences::new();
        //let s  = serde_json::to_string(graph.settings).unwrap();
        let js = "{\"name\": \"Hello\", \"val\": 42}";
        let test : Vec<Box<Display>> = vec![Box::new(1), Box::new("string")];
        //let msg = serde_msgpack::to_vec(serde_json::from_str(js).unwrap()).unwrap();
        let v: Result<Name,_> = serde_json::from_str(js);
        match v {
            Ok(v) => println!("Result: {}", v.name),
            Err(e) => println!("Error: {}", e),
        };
        for i in test {
            println!("Val: {}", i);
        }
        
        control.connect(|s| { println!("String: {:?}", s); Vec::from("rval")} );
        Ok(Context { name,
                     table,
                     format,
                     probes: vec![],
                     control,
                     structure,
                     graph,
                     preferences })
    }

    pub fn run(&self) {
        loop {
            thread::sleep_ms(1000);
        }
    }
}
