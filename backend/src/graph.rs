use gst::prelude::*;
use gst;
use std::sync::{Arc,Mutex};
use std::sync::mpsc::Sender;
use signals::Signal;
use root::Root;
use branch::Typ;
use wm::Wm;
use wm::template::{WmTemplate,WmTemplatePartial};
use metadata::Structure;
use settings::{Settings,SettingsFlat};
//use audio_mux::Mux;
use renderer::{VideoR,AudioR,Renderer};

pub struct GraphState {
    sender_data: Arc<Mutex<Sender<(Typ,String,u32,u32,gst::Buffer)>>>,
    sender_status: Arc<Mutex<Sender<(String,u32,u32,bool)>>>,
    settings:      Settings,
    pub structure: Vec<Structure>,

    pipeline:    gst::Pipeline,
    bus:         gst::Bus,
    pub wm:      Arc<Mutex<Wm>>,
    // pub mux:     Arc<Mutex<Mux>>,
    roots:       Vec<Root>,
    vrend:       Option<Renderer<VideoR>>,
    arend:       Option<Renderer<AudioR>>,
}

pub struct Graph {
    sender:  Arc<Mutex<Sender<Vec<u8>>>>,
    state:   Arc<Mutex<GraphState>>,
}

impl GraphState {
    pub fn new (sender_wm: Sender<Vec<u8>>,
                sender_data: Sender<(Typ,String,u32,u32,gst::Buffer)>,
                sender_status: Sender<(String,u32,u32,bool)>)
                -> GraphState {
        let sender_data = Arc::new(Mutex::new(sender_data));
        let sender_status = Arc::new(Mutex::new(sender_status));
        let settings  = Settings::new();
        let structure = Vec::new();
        let pipeline  = gst::Pipeline::new(None);
        let wm        = Arc::new(Mutex::new(Wm::new(sender_wm)));
        //let mux       = Arc::new(Mutex::new(Mux::new(sender.clone())));
        let vrend     = None;
        let arend     = None;
        let bus       = pipeline.get_bus().unwrap();
        let roots     = Vec::new();
        wm.lock().unwrap().init(&pipeline);
        //mux.lock().unwrap().init(&pipeline);
        GraphState { sender_data, sender_status,
                     settings, structure, pipeline, wm,
                     /*mux,*/ bus, roots, arend, vrend }
    }

    pub fn reset (&mut self) {
        debug!("GraphState::reset [pipeline pause]");
        gst::debug_bin_to_dot_file(&self.pipeline, gst::DebugGraphDetails::VERBOSE, "pipeline_pre_reset");
        let _ = self.pipeline.set_state(gst::State::Null);
        debug!("GraphState::reset [pipeline refcounter] {}", self.pipeline.ref_count());
        
        debug!("GraphState::reset [pipeline reset]");
        self.pipeline = gst::Pipeline::new(None);
        debug!("GraphState::reset [bus reset]");
        self.bus      = self.pipeline.get_bus().unwrap();
        debug!("GraphState::reset [root reset]");
        self.roots    = Vec::new();       
        debug!("GraphState::reset [vrend reset]");
        self.vrend    = None;
        debug!("GraphState::reset [arend reset]");
        self.arend    = None;
        debug!("GraphState::reset [audio mux reset]");
        //self.mux.lock().unwrap().reset();
        debug!("GraphState::reset [wm reset]");
        self.wm.lock().unwrap().reset();
        gst::debug_bin_to_dot_file(& self.pipeline, gst::DebugGraphDetails::VERBOSE, "pipeline_post_reset");
    }

    pub fn init (&mut self) {
        debug!("GraphState::init");
        self.wm.lock().unwrap().init(&self.pipeline);
        //self.mux.lock().unwrap().init(&self.pipeline);
        self.vrend    = Some(Renderer::<VideoR>::new(5004, &self.pipeline));
        self.arend    = Some(Renderer::<AudioR>::new(5005, &self.pipeline));
        debug!("GraphState::reset [video render reset]");
        self.vrend.iter().for_each(|rend| rend.plug(&self.wm.lock().unwrap().src_pad()));
        debug!("GraphState::reset [audio render reset]");
        //self.arend.iter().for_each(|rend| rend.plug(&self.mux.lock().unwrap().src_pad()));
    }

    pub fn set_state (&self, st: gst::State) {
        // TODO check
        debug!("GraphState::set_state [{:?}]", st);
        let _ = self.pipeline.set_state(st);
    }

    pub fn apply_streams (&mut self, s: Vec<Structure>) -> Result<(),String> {
        debug!("Graph::apply_streams [reset]");
        self.reset();
        self.init();
        debug!("Graph::apply_streams [loop]");

        for stream in &s {
            if let Some(mut root) = Root::new(&self.pipeline,
                                              &stream,
                                              &self.sender_data,
                                              &self.sender_status) {
                //let pipe   = self.pipeline.clone();
                let wm     = Arc::downgrade(&self.wm);
                //let mux    = self.mux.clone();
                root.pad_added.lock().unwrap().connect(move |p| {
                    debug!("Graph::apply_streams [attach pad {} {}]", p.stream, p.channel);
                    match wm.upgrade () {
                        None => (),
                        Some (ref wm) =>
                            wm.lock().unwrap().plug(p),
                    }
                    // TODO check
                    //let _ = pipe.set_state(gst::State::Playing);
                    //gst::debug_bin_to_dot_file(&pipe, gst::DebugGraphDetails::VERBOSE, "pipeline");
                });
                
                root.audio_pad_added.lock().unwrap().connect(move |p| {
                    //mux.lock().unwrap().plug(p);
                });

                root.apply_settings(&self.settings);
            }
        };
        // TODO replace with retain_state
        
        let _ = self.pipeline.set_state(gst::State::Playing);
        self.structure = s;
        Ok(())
    }

    pub fn apply_settings (&mut self, s: Settings) -> Result<(),String> {
        debug!("Graph::apply_settings");
        self.roots.iter_mut().for_each(|root : &mut Root| root.apply_settings(&s) );
        self.settings = s;
        Ok(())
    }
}

impl Graph {
    
    pub fn new (sender_graph: Sender<Vec<u8>>,
                sender_wm: Sender<Vec<u8>>,
                sender_data: Sender<(Typ,String,u32,u32,gst::Buffer)>,
                sender_status: Sender<(String,u32,u32,bool)>)
                -> Result<Graph,String> {
        let sender = Arc::new(Mutex::new( sender_graph ));
        let state  = Arc::new(Mutex::new( GraphState::new(sender_wm,
                                                          sender_data,
                                                          sender_status) ));
        Ok(Graph { sender, state } )
    }
/*
    pub fn get_wm (&self) -> Arc<Mutex<Wm>> {
        self.state.lock().unwrap().wm.clone()
    }
*/
    pub fn get_structure (&self) -> Vec<Structure> {
        self.state.lock().unwrap().structure.clone()
    }

    pub fn set_structure (&self, s: Vec<Structure>) -> Result<(),String> {
        // TODO talk only when applied successfully
        self.sender.lock().unwrap().send(serde_json::to_vec(&s).unwrap());
        self.state.lock().unwrap().apply_streams(s)
        //if res.is_ok() {
        //    self.chat.lock().unwrap().talk()
        //};
    }

    pub fn get_wm_layout (&self) -> Result<WmTemplate,String> {
        match self.state.lock() {
            Ok(s)  => s.wm.lock().unwrap().get_layout(),
            Err(_) => Err(String::from("can't acquire wm state")), 
        }
    }

    pub fn set_wm_layout (&self, templ: WmTemplatePartial) -> Result<(),String> {
        match self.state.lock() {
            Ok(s)  => s.wm.lock().unwrap().set_layout(templ),
            Err(_) => Err(String::from("can't acquire wm state")), 
        }
    }

    pub fn get_settings (&self) -> Result<SettingsFlat,String> {
        let s = &self.state.lock().unwrap().settings;
        Ok(s.to_flat())
    }

    pub fn set_settings (&self, sf: SettingsFlat) -> Result<(),String> {
        let s = Settings::from_flat(sf);
        self.state.lock().unwrap().apply_settings(s)
    }

            /*
    pub fn connect_destructive (&mut self, msg: &mut Msg<Vec<Structure>,Result<(),String>>) {
        let state  = self.state.clone();
        let notif  = self.chat.clone();
        msg.connect(move |s| {
            debug!("Graph::destructive");
            let res = state.lock().unwrap().apply_streams(&s);
            if res.is_ok() {
                debug!("Graph::destructive [send state]");
                notif.lock().unwrap().talk(&s)
            };
            res
        }).unwrap();
    }

    pub fn connect_settings (&mut self, msg: &mut Msg<Settings,Result<(),String>>) {
        let state  = self.state.clone();
        msg.connect(move |s| {
            debug!("Graph::settings");
            state.lock().unwrap().apply_settings(s)
        }).unwrap();
    }
         */
    
}
