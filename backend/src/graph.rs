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

use glib::translate::ToGlibPtr;
use std::ffi::{CString, CStr};

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

unsafe fn string_to_chars (s: &str) -> *mut libc::c_char {
    let size = s.len();    
    //let cstr = CString::new(res).unwrap();
    let buf = libc::calloc(size + 1, 1);
    libc::memcpy(buf, s.as_ptr() as *const libc::c_void, size);
    buf as *mut libc::c_char
}

unsafe extern "C" fn on_destroy (p : glib_sys::gpointer) {
    let c = CStr::from_ptr(p as *mut libc::c_char);
    error!("GraphState::reset [pipeline] element {} was destroyed", c.to_str().unwrap());
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
        {
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

        let c_str = CString::new("destroy").unwrap();
        debug!("GraphState::reset [pipeline refcounter] {}", self.pipeline.ref_count());
        for el in self.pipeline.iterate_recurse() {
            match el {
                Err (_) => error!("GraphState::reset [pipeline] Unknown iter error"),
                Ok (e) => {
                    error!("GraphState::reset [pipeline] element {} counter {}",
                           e.get_name(),
                           e.ref_count());
                    unsafe {
                        let name = string_to_chars(&e.get_name().as_str());
                        let c : *mut gst_sys::GstElement = e.to_glib_full();
                        gstreamer_sys::gst_object_unref(c as *mut gst_sys::GstObject);
                        gobject_sys::g_object_set_data_full(c as *mut gobject_sys::GObject,
                                                            c_str.as_ptr() as *const libc::c_char,
                                                            name as glib_sys::gpointer,
                                                            Some(on_destroy));
                        let p = gobject_sys::g_object_get_data (c as *mut gobject_sys::GObject,
                                                                c_str.as_ptr() as *const libc::c_char);
                        let v = CStr::from_ptr(p as *mut libc::c_char);
                        error!("Data was attached to {}", v.to_str().unwrap());
                    }
                }
            }
        }

        unsafe {
            let p : *mut gst_sys::GstPipeline = self.pipeline.to_glib_full();
            gobject_sys::g_object_set_data_full(p as *mut gobject_sys::GObject,
                                                c_str.as_ptr() as *const libc::c_char,
                                                p as glib_sys::gpointer,
                                                Some(on_destroy));
        }

        {
            debug!("GraphState::reset [pipeline reset]");
            self.pipeline = gst::Pipeline::new(None);
        }
        debug!("GraphState::reset [bus reset]");
        self.bus      = self.pipeline.get_bus().unwrap();
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

    pub fn reset (&self) {
        self.state.lock().unwrap().reset()
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
