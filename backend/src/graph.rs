use chatterer::MsgType;
use chatterer::notif::Notifier;
use chatterer::control::{Addressable,Replybox};
use std::sync::{Arc,Mutex};
use std::sync::mpsc::Sender;
use signals::Signal;
use gst::prelude::*;

#[derive(Serialize,Deserialize,Debug)]
pub struct GraphSettings {
    state: String,
}

pub struct Graph {
    format:      MsgType,
    pub chat:    Arc<Mutex<Notifier>>,
    settings:    GraphSettings,
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
