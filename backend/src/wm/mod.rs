pub mod position;
pub mod widget;
pub mod widget_video;
pub mod widget_soundbar;
pub mod widget_factory;
pub mod template;

use std::collections::HashMap;
use std::sync::{Arc,Mutex};
use std::sync::mpsc::Sender;
use gst::prelude::*;
use gst;
use glib;
use signals::Signal;
use pad::{Type,SrcPad};
use wm::widget::Widget;
use wm::position::Position;
use wm::template::{WmTemplate,WmTemplatePartial,ContainerTemplate};

pub struct Container {
    pub position: Position,
    pub widgets:  HashMap<String,Arc<Mutex<Widget + Send>>>, // TODO WeakRef
}

pub struct WmState {    
    resolution: (u32, u32),
    layout:     HashMap<String,Container>,
    widgets:    HashMap<String,Arc<Mutex<Widget + Send>>>,

    pipe:           glib::WeakRef<gst::Pipeline>,
    caps:           gst::Element,
    download:       gst::Element,
    mixer:          gst::Element,
}

pub struct Wm {
    sender:  Arc<Mutex<Sender<Vec<u8>>>>,
    state:   Arc<Mutex<Option<WmState>>>,
}

fn enum_to_val(cls: &str, val: i32) -> glib::Value {
    glib::EnumClass::new(glib::Type::from_name(cls).unwrap()).unwrap().to_value(val).unwrap()
}

impl WmState {
    fn resolution_caps ((width,height) : (u32, u32)) -> String {
        format!("video/x-raw(ANY),height={},width={}", height, width)
    }
    
    pub fn new (pipe: &gst::Pipeline, resolution: (u32, u32)) -> WmState {
        let caps     = gst::ElementFactory::make("capsfilter", None).unwrap();
        let download = gst::ElementFactory::make("gldownload", None).unwrap();
        let mixer    = gst::ElementFactory::make("glvideomixer", None).unwrap();
        
        mixer.set_property("background", &enum_to_val("GstGLVideoMixerBackground", 1)).unwrap();
        mixer.set_property("async-handling", &true).unwrap();
        mixer.set_property("latency", &100_000_000u64).unwrap();
        caps.set_property("caps", &gst::Caps::from_string(& WmState::resolution_caps(resolution)).unwrap()).unwrap();

        pipe.add_many(&[&mixer,&caps,&download]).unwrap();
        mixer.link(&caps).unwrap();
        caps.link(&download).unwrap();

        let pipe = pipe.downgrade();
        
        WmState { resolution, layout: HashMap::new(), widgets: HashMap::new(),
                  pipe, caps, download, mixer }
    }

    pub fn plug (&mut self, pad: &SrcPad) -> Option<Arc<Mutex<Signal<()>>>> {
        let pipe = self.pipe.upgrade().unwrap();

        match pad.typ {
            Type::Video => {
                let uid;
                let widg = widget_factory::make("video").unwrap();
                {
                    let mut widg = widg.lock().unwrap();
                    widg.add_to_pipe(&pipe.upcast());
                    let sink_pad = self.mixer.get_request_pad("sink_%u").unwrap();
                    widg.plug_src(pad);
                    widg.plug_sink(sink_pad);
                    uid = widg.gen_uid();
                }
                let signal = widg.lock().unwrap().linked();
                self.widgets.insert(uid, widg);
                Some(signal)
            }
            Type::Audio => {
                let uid;
                let widg = widget_factory::make("audio").unwrap();
                {
                    let mut widg = widg.lock().unwrap();
                    widg.add_to_pipe(&pipe.upcast());
                    let sink_pad = self.mixer.get_request_pad("sink_%u").unwrap();
                    widg.plug_src(pad);
                    widg.plug_sink(sink_pad);
                    uid = widg.gen_uid();
                }
                let signal = widg.lock().unwrap().linked();
                self.widgets.insert(uid, widg);
                Some(signal)
            }
            _ => None
        }
    }

    pub fn set_resolution (&mut self, res: (u32, u32)) {
        self.caps.set_property("caps", &gst::Caps::from_string(& WmState::resolution_caps(res))
                               .unwrap())
            .unwrap();
        self.resolution = res;
    }
    
    pub fn from_template (&mut self, t: &WmTemplate) -> Result<(),String> {
        let pipe = self.pipe.upgrade().unwrap();
        for &(ref name,_) in &t.widgets {
            if ! self.widgets.keys().any(|n| n == name) {
                return Err(format!("Wm: Widget {} does not exists", name))
            }
        }
        t.validate()?;
        self.widgets.iter_mut().for_each(|(_,w)| w.lock().unwrap().disable());

        self.layout = HashMap::new();
        for &(ref cname, ref c) in &t.layout {
            let res = (t.resolution.0 as f64, t.resolution.1 as f64);
            let container_position = c.position.to_absolute(res);
            let offset   = (container_position.get_x(), container_position.get_y());
            let mut widgets  = HashMap::new();
            for &(ref wname, ref w) in &c.widgets {
                let widget = self.widgets[wname].clone();
                let layer    = w.layer;
                let position = match w.position {
                    None => container_position,
                    Some (pos) => {
                        let parent = (container_position.w, container_position.h);
                        pos.to_absolute(parent)
                    }
                };
                widget.lock().unwrap().render(offset, position, layer);
                widgets.insert(wname.clone(), widget);
            }
            self.layout.insert(cname.clone(),
                               Container { position: c.position, widgets } );
        }
        self.set_resolution(t.resolution);
        let _ = pipe.set_state(gst::State::Playing); // TODO
        gst::debug_bin_to_dot_file(&pipe, gst::DebugGraphDetails::VERBOSE, "wm");
        Ok(())
    }
    
    pub fn to_template (&self) -> WmTemplate {
        let resolution = self.resolution;
        let layout     = self.layout.iter()
            .map(|(cname,c)| {
                let position = c.position;
                let widgets  = c.widgets.iter()
                    .map(move |(wname,w)| (wname.clone(), w.lock().unwrap().get_desc()))
                    .collect();
                let cont = ContainerTemplate { position, widgets };
                (cname.clone(), cont)
            })
            .collect();
        let widgets    = self.widgets.iter()
            .map(move |(wname,w)| (wname.clone(), w.lock().unwrap().get_desc().clone()))
            .collect();
        WmTemplate { resolution, layout, widgets }
    }
}

impl Wm {
    pub fn new (sender: Sender<Vec<u8>>) -> Wm {
        let sender = Arc::new(Mutex::new( sender ));
        let state = Arc::new(Mutex::new( None ));
        Wm { sender, state }
    }

    pub fn init (&mut self, pipe: &gst::Pipeline) {
        let wm = WmState::new(pipe, (1280,720));
        self.sender.lock().unwrap().send(serde_json::to_vec(&wm.to_template()).unwrap());
        *self.state.lock().unwrap() = Some(wm);
    }
    
    pub fn reset (&mut self) {
        *self.state.lock().unwrap() = None;
    }

    pub fn plug (&mut self, pad: &SrcPad) {
        match *self.state.lock().unwrap() {
            None => (), // TODO invariant?
            Some (ref mut state) => 
                if let Some(linked) = state.plug(&pad) {
                    let sender = Arc::downgrade (&self.sender);
                    let state  = Arc::downgrade (&self.state);
                    // TODO cleanup
                    linked.lock().unwrap().connect(move |&()| {
                        match state.upgrade() {
                            None => (),
                            Some (ref state) =>
                                match sender.upgrade () {
                                    None => (),
                                    Some (ref sender) => {
                                        let data = &*state.lock().unwrap();
                                        match data {
                                            None => (), //TODO log
                                            Some (v) => 
                                                sender.lock().unwrap()
                                                .send (serde_json::to_vec(&v.to_template())
                                                       .unwrap())
                                                .unwrap()
                                        }
                                    },
                                },
                        }
                    });
                }
        }
    }

    pub fn src_pad (&self) -> gst::Pad {
        match *self.state.lock().unwrap() {
            None => panic!("Wm::src_pad invariant is brocken"), // TODO invariant?
            Some(ref state) => state.download.get_static_pad("src").unwrap()
        }
    }

    pub fn get_layout (&self) -> Result<WmTemplate,String> {
        match *self.state.lock().unwrap() {
            None => Err(String::from("Wm is not initialized")),
            Some(ref wm) => Ok(wm.to_template()),
        }
    }

    pub fn set_layout (&self, templ: WmTemplatePartial) -> Result<(),String> {
        match *self.state.lock().unwrap() {
            None => Err(String::from("Wm is not initialized")),
            Some(ref mut wm) => {
                let widg = wm.widgets.iter()
                    .map(move |(name,w)| (name.clone(), w.lock().unwrap().get_desc().clone()))
                    .collect();
                let temp = WmTemplate::from_partial(templ, &widg);
                wm.from_template(&temp)
                // TODO send event
            }
        }
    }
}
