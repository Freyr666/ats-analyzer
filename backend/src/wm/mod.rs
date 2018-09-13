pub mod position;
pub mod widget;
pub mod widget_video;
pub mod widget_soundbar;
pub mod widget_factory;
pub mod template;

use serde::Serialize;
use std::collections::HashMap;
use std::sync::{Arc,Mutex};
use std::rc::Rc;
use std::sync::mpsc::Sender;
use gst::prelude::*;
use gst;
use glib;
use chatterer::notif::Notifier;
use chatterer::control::{Addressable,Replybox};
use chatterer::control::message::{Request,Reply};
use chatterer::MsgType;
use signals::Signal;
use pad::{Type,SrcPad};
use wm::widget::{Widget,WidgetDesc};
use wm::position::Position;
use wm::template::{WmTemplate,WmTemplatePartial,ContainerTemplate};

pub struct Container {
    pub position: Position,
    pub widgets:  HashMap<String,Arc<Mutex<Widget + Send>>>,
}

pub struct WmState {    
    resolution: (u32, u32),
    layout:     HashMap<String,Container>,
    widgets:    HashMap<String,Arc<Mutex<Widget + Send>>>,

    pipe:           gst::Pipeline,
    caps:           gst::Element,
    download:       gst::Element,
    mixer:          gst::Element,
}

pub struct Wm {
    format:      MsgType,
    pub chat:    Arc<Mutex<Notifier>>,

    state:       Arc<Mutex<WmState>>,
}

fn enum_to_val(cls: &str, val: i32) -> glib::Value {
    glib::EnumClass::new(glib::Type::from_name(cls).unwrap()).unwrap().to_value(val).unwrap()
}

impl WmState {
    fn resolution_caps ((width,height) : (u32, u32)) -> String {
        format!("video/x-raw(ANY),height={},width={}", height, width)
    }
    
    pub fn new (pipe: gst::Pipeline, resolution: (u32, u32)) -> WmState {
        let caps     = gst::ElementFactory::make("capsfilter", None).unwrap();
        let download = gst::ElementFactory::make("gldownload", None).unwrap();
        let mixer    = gst::ElementFactory::make("glvideomixer", None).unwrap();
        
        mixer.set_property("background", &enum_to_val("GstGLVideoMixerBackground", 1)).unwrap();
        mixer.set_property("async-handling", &true).unwrap();
        mixer.set_property("latency", &100000000i64).unwrap();
        caps.set_property("caps", &gst::Caps::from_string(& WmState::resolution_caps(resolution)).unwrap()).unwrap();

        pipe.add_many(&[&mixer,&caps,&download]).unwrap();
        mixer.link(&caps).unwrap();
        caps.link(&download).unwrap();
        
        WmState { resolution, layout: HashMap::new(), widgets: HashMap::new(),
                  pipe, caps, download, mixer }
    }

    pub fn plug (&mut self, pad: &SrcPad) -> Option<Arc<Mutex<Signal<()>>>> {
        match pad.typ {
            Type::Video => {
                let uid;
                let widg = widget_factory::make("video").unwrap();
                {
                    let mut widg = widg.lock().unwrap();
                    widg.add_to_pipe(self.pipe.clone().upcast());
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
                    widg.add_to_pipe(self.pipe.clone().upcast());
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
                               .unwrap()).unwrap();
        self.resolution = res;
    }
    
    pub fn from_template (&mut self, t: &WmTemplate) -> Result<(),String> {
        for &(ref name,_) in &t.widgets {
            if ! self.widgets.keys().any(|n| n == name) {
                return Err(format!("Wm: Widget {} does not exists", name))
            }
        }
        t.validate()?;
        self.widgets.iter_mut().for_each(|(_,w)| w.lock().unwrap().set_enable(false));

        self.layout = HashMap::new();
        for &(ref cname, ref c) in &t.layout {
            let position = c.position;
            let mut widgets  = HashMap::new();
            for &(ref wname, ref w) in &c.widgets {
                let widget = self.widgets.get(wname).unwrap().clone();
                widget.lock().unwrap().apply_desc(&w);
                widget.lock().unwrap().set_enable(true);
                widgets.insert(wname.clone(), widget);
            }
            self.layout.insert(cname.clone(), Container { position, widgets } );
        }
        self.set_resolution(t.resolution);
        self.pipe.set_state(gst::State::Playing);
        gst::debug_bin_to_dot_file(&self.pipe, gst::DebugGraphDetails::VERBOSE, "wm");
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

impl Addressable for Wm {
    fn get_name (&self) -> &str { "wm" }
    fn get_format (&self) -> MsgType { self.format }
}

impl Replybox<Request<WmTemplatePartial>,Reply<WmTemplate>> for Wm {
    
    fn reply (&self) ->
        Box<Fn(Request<WmTemplatePartial>)->Result<Reply<WmTemplate>,String> + Send + Sync> {
            let state = self.state.clone();
            let auxilary = move |templ: WmTemplatePartial| -> Result<(),String> {
                let widg = state.lock().unwrap().widgets.iter()
                    .map(move |(name,w)| (name.clone(), w.lock().unwrap().get_desc().clone()))
                    .collect();
                let temp = WmTemplate::from_partial(templ, widg);
                state.lock().unwrap().from_template(&temp)
            };
            
            let state = self.state.clone();
            Box::new(move |req| {
                match req {
                    Request::Get =>
                        if let Ok(s) = state.lock() {
                            Ok(Reply::Get(s.to_template()))
                        } else {
                            Err(String::from("can't acquire wm layout"))
                        },
                    Request::Set(templ) => match auxilary(templ) {
                            Ok(()) => Ok(Reply::Set),
                            Err(e) => Err(e),
                    }
                }
            })
        }
}

impl Wm {
    pub fn new (pipe: gst::Pipeline, format: MsgType, sender: Sender<Vec<u8>>) -> Wm {
        let chat = Arc::new(Mutex::new( Notifier::new("wm", format, sender )));
        let state = Arc::new(Mutex::new(WmState::new(pipe, (1280,720)) ));
        Wm { format, chat, state }
    }

    pub fn reset (&mut self, pipe: gst::Pipeline) {
        *self.state.lock().unwrap() = WmState::new(pipe, (1280,720));
        self.chat.lock().unwrap().talk(&self.state.lock().unwrap().to_template());
    }

    pub fn src_pad (&self) -> gst::Pad {
        self.state.lock().unwrap().download.get_static_pad("src").unwrap()
    }

    pub fn plug (&mut self, pad: &SrcPad) {
        if let Some(linked) = self.state.lock().unwrap().plug(&pad) {
            let chat  = self.chat.clone();
            let state = self.state.clone();
            linked.lock().unwrap().connect(move |&()| {
                chat.lock().unwrap().talk(&state.lock().unwrap().to_template());
            });
        }
    }
}
