use chatterer::MsgType;
use chatterer::notif::Notifier;
use chatterer::control::{Addressable,Replybox};
use root::Root;
use std::sync::{Arc,Mutex};
use std::sync::mpsc::Sender;
use signals::Signal;
use gst::prelude::*;
use gst;

#[derive(Serialize,Deserialize,Debug)]
pub struct GraphSettings {
    state: String,
}

pub struct Graph {
    format:      MsgType,
    pub chat:    Arc<Mutex<Notifier>>,

    settings:    GraphSettings,
    pipeline:    gst::Pipeline,
    bus:         gst::Bus,
    roots:       Vec<Root>,
}

impl Addressable for Graph {
    fn get_name(&self) -> &str { "graph" }
    fn get_format(&self) -> MsgType { self.format }
}

impl Replybox<String,String> for Graph {
    fn reply (&self) -> Box<Fn(String)->Result<String,String> + Send + Sync> {
        Box::new(move |name| {
            let s = String::from("Hello, ");
            Ok(s + &name)
        })
    }
}

impl Graph {
    
    pub fn new(format: MsgType, sender: Sender<Vec<u8>>) -> Result<Graph,String> {
        let settings = GraphSettings { state: String::from("") };
        let chat = Arc::new(Mutex::new( Notifier::new("graph", format, sender )));
        Ok(Graph { chat, settings, format } )
    }

    
}
