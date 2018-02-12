use chatterer::MsgType;
use chatterer::notif::Notifier;
use chatterer::control::{Addressable,Replybox};
use root::Root;
use wm::Wm;
use pad::{Type,SrcPad};
use std::sync::{Arc,Mutex};
use std::sync::mpsc::Sender;
use signals::{Signal,Msg};
use metadata::Structure;
use gst::prelude::*;
use gst;

#[derive(Serialize,Deserialize,Debug)]
pub struct GraphSettings {
    state: String,
}

pub struct GraphState {
    settings:    GraphSettings,
    pipeline:    gst::Pipeline,
    bus:         gst::Bus,
    pub wm:      Arc<Mutex<Wm>>,
    roots:       Vec<Root>,
}

pub struct Graph {
    format:      MsgType,
    pub chat:    Arc<Mutex<Notifier>>,

    state:       Arc<Mutex<GraphState>>,
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

impl GraphState {
    pub fn new (format: MsgType, sender: Sender<Vec<u8>>) -> GraphState {
        let settings = GraphSettings { state: String::from("") };
        let pipeline = gst::Pipeline::new(None);
        let wm       = Arc::new(Mutex::new(Wm::new(pipeline.clone(), format, sender)));
        let bus      = pipeline.get_bus().unwrap();
        let roots    = Vec::new();
        GraphState { settings, pipeline, wm, bus, roots }
    }

    pub fn reset (&mut self) {
        self.pipeline.set_state(gst::State::Null);
        self.roots    = Vec::new();
        self.pipeline = gst::Pipeline::new(None);
        self.wm.lock().unwrap().reset(self.pipeline.clone());
        self.bus      = self.pipeline.get_bus().unwrap();
    }

    pub fn set_state (&self, st: gst::State) {
        self.pipeline.set_state(st);
    }

    pub fn apply_streams (&mut self, s: Vec<Structure>) -> Result<(),String> {
        println!("Apply Stream");
        self.reset();

        for s in s.iter() {
            println!("Stream");
            if let Some(root) = Root::new(self.pipeline.clone().upcast(), s.clone()) {
                println!("New root");
                let pipe = self.pipeline.clone();
                let wm = self.wm.clone();
                root.pad_added.lock().unwrap().connect(move |p| {
                    println!("Pad added");
                    wm.lock().unwrap().plug(p);
                    //pipe.set_state(gst::State::Playing);
                    gst::debug_bin_to_dot_file(&pipe, gst::DebugGraphDetails::VERBOSE, "pipeline");
                });
            }
        };
        self.pipeline.set_state(gst::State::Playing);
        Ok(())
    }
}

impl Graph {
    
    pub fn new(format: MsgType, sender: Sender<Vec<u8>>) -> Result<Graph,String> {
        let chat = Arc::new(Mutex::new( Notifier::new("graph", format, sender.clone() )));
        let state = Arc::new(Mutex::new(GraphState::new(format, sender) ));
        Ok(Graph { format, chat, state } )
    }

    pub fn connect_destructive (&mut self, msg: &mut Msg<Vec<Structure>,Result<(),String>>) {
        let state = self.state.clone();
        msg.connect(move |s| {
            println!("Got structs: {:?}", s);
            state.lock().unwrap().apply_streams(s)
        }).unwrap();
    }
    
}
