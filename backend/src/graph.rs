use chatterer::MsgType;
use chatterer::{Addressable,Replybox};
use std::sync::{Arc,Mutex};
use std::sync::mpsc::Sender;
use signals::Signal;
use gst::prelude::*;

#[derive(Serialize,Deserialize,Debug)]
pub struct GraphSettings {
    state: String,
}

pub struct GraphState {
    format:     MsgType,
    sender:     Option<Sender<Vec<u8>>>,
    
    settings:   GraphSettings,
}

pub struct Graph {
    pub state:   Arc<Mutex<GraphState>>
}

impl Addressable for GraphState {
    fn get_name(&self) -> &str { "graph" }
    fn get_format(&self) -> MsgType { self.format }
    fn set_format(&mut self, f: MsgType) { self.format = f }
}

impl Replybox<String,String> for GraphState {
    fn reply (&self) -> Box<Fn(String)->Result<String,String> + Send + Sync> {
        Box::new(move |name| {
            let s = String::from("Hello, ");
            Ok(s + &name)
        })
    }
}

impl Graph {
    pub fn new() -> Result<Graph,String> {
        let settings = GraphSettings { state: String::from("") };
        let state = Arc::new(Mutex::new( GraphState { format: MsgType::Json, sender: None, settings } ));
        Ok(Graph { state } )
    }

    //pub fn run();
}

// impl<'a,GraphSettings> Chatterer<'a,GraphSettings> for Graph<'a>
//     where GraphSettings: Serialize+Deserialize<'a>{

//     fn name(&self) -> &'static str {
//         "graph"
//     }

//     fn ask_state(&'a self) -> &'a GraphSettings {
//         let a : &'a GraphSettings = self.settings;
//         a
//     }

//     fn set_state(&self, new_state: &GraphSettings) -> Response {
//         Response::Fine
//     }

//     fn signal(&self) -> Arc<Mutex<Signal<&'a GraphSettings>>> {
//         self.signal
//     }
// }
