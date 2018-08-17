use gst::prelude::*;
use gst;
use std::sync::{Arc,Mutex};
use std::sync::mpsc::Sender;
use chatterer::MsgType;
use chatterer::notif::Notifier;
use chatterer::control::{Addressable,Replybox};
use root::Root;
use wm::Wm;
use pad::{Type,SrcPad};
use signals::{Signal,Msg};
use metadata::Structure;
use settings::Settings;
use audio_mux::Mux;
use renderer::{VideoR,AudioR,Renderer};

pub struct GraphState {
    format:      MsgType,
    sender:      Sender<Vec<u8>>,
    settings:    Option<Settings>,
    pipeline:    gst::Pipeline,
    bus:         gst::Bus,
    pub wm:      Arc<Mutex<Wm>>,
    pub mux:     Arc<Mutex<Mux>>,
    roots:       Vec<Root>,
    vrend:       Option<Renderer<VideoR>>,
    arend:       Option<Renderer<AudioR>>,
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
        let settings = None;
        let pipeline = gst::Pipeline::new(None);
        let wm       = Arc::new(Mutex::new(Wm::new(pipeline.clone(), format, sender.clone())));
        let mux      = Arc::new(Mutex::new(Mux::new(pipeline.clone(), format, sender.clone())));
        let vrend    = None;
        let arend    = None;
        let bus      = pipeline.get_bus().unwrap();
        let roots    = Vec::new();
        GraphState { settings, pipeline, wm, mux, bus, roots, arend, vrend, format, sender }
    }

    pub fn reset (&mut self) {
        self.pipeline.set_state(gst::State::Null);
        self.roots    = Vec::new();
        self.pipeline = gst::Pipeline::new(None);
        self.wm.lock().unwrap().reset(self.pipeline.clone());
        self.mux.lock().unwrap().reset(self.pipeline.clone());
        self.vrend    = Some(Renderer::<VideoR>::new(5004, self.pipeline.clone().upcast()));
        self.arend    = Some(Renderer::<AudioR>::new(5005, self.pipeline.clone().upcast()));
        self.bus      = self.pipeline.get_bus().unwrap();

        self.vrend.iter().for_each(|rend| rend.plug(self.wm.lock().unwrap().src_pad().clone()));
        self.arend.iter().for_each(|rend| rend.plug(self.mux.lock().unwrap().src_pad().clone()));
    }

    pub fn set_state (&self, st: gst::State) {
        self.pipeline.set_state(st);
    }

    pub fn apply_streams (&mut self, s: Vec<Structure>) -> Result<(),String> {
        //println!("Apply Stream");
        self.reset();
        
        for s in s.iter() {
            //println!("Stream");
            if let Some(root) = Root::new(self.pipeline.clone().upcast(), s.clone(),
                                          self.settings.clone(),
                                          self.format, self.sender.clone()) {
                //println!("New root");
                let pipe   = self.pipeline.clone();
                let wm     = self.wm.clone();
                let mux    = self.mux.clone();
                let apipe  = self.pipeline.clone();
                root.pad_added.lock().unwrap().connect(move |p| {
                    //println!("Pad added");
                    wm.lock().unwrap().plug(p);
                    pipe.set_state(gst::State::Playing);
                    //gst::debug_bin_to_dot_file(&pipe, gst::DebugGraphDetails::VERBOSE, "pipeline");
                });
                
                root.audio_pad_added.lock().unwrap().connect(move |p| {
                    mux.lock().unwrap().plug(p);
                });
            }
        };
        // TODO replace with retain_state
        self.pipeline.set_state(gst::State::Playing);
        Ok(())
    }

    pub fn apply_settings (&mut self, s: Settings) -> Result<(),String> {
        self.settings = Some(s);
        self.roots.iter_mut().for_each(|root : &mut Root| root.apply_settings(s) );
        Ok(())
    }
}

impl Graph {
    
    pub fn new (format: MsgType, sender: Sender<Vec<u8>>) -> Result<Graph,String> {
        let chat = Arc::new(Mutex::new( Notifier::new("graph", format, sender.clone() )));
        let state = Arc::new(Mutex::new(GraphState::new(format, sender.clone()) ));
        Ok(Graph { format, chat, state } )
    }

    pub fn get_wm (&self) -> Arc<Mutex<Wm>> {
        self.state.lock().unwrap().wm.clone()
    }
    
    pub fn connect_destructive (&mut self, msg: &mut Msg<Vec<Structure>,Result<(),String>>) {
        let state  = self.state.clone();
        msg.connect(move |s| {
            state.lock().unwrap().apply_streams(s)
        }).unwrap();
    }

    pub fn connect_settings (&mut self, msg: &mut Msg<Settings,Result<(),String>>) {
        let state  = self.state.clone();
        msg.connect(move |s| {
            state.lock().unwrap().apply_settings(s)
        }).unwrap();
    }
    
}
