pub mod position;
pub mod widget;
pub mod widget_video;
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
use chatterer::MsgType;
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
    background:     gst::Element,
    background_pad: gst::Pad,
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
    pub fn new (pipe: gst::Pipeline, resolution: (u32, u32)) -> WmState {
        let background     = gst::ElementFactory::make("videotestsrc", None).unwrap();
        let mixer          = gst::ElementFactory::make("compositor", None).unwrap();
        background.set_property("is_live", &true).unwrap();
        background.set_property("pattern", &enum_to_val("GstVideoTestSrcPattern", 2)).unwrap();
        mixer.set_property("background", &enum_to_val("GstCompositorBackground", 1)).unwrap();
        pipe.add_many(&[&background,&mixer]).unwrap();
        
        let background_pad = mixer.get_request_pad("sink_%u").unwrap();
        background_pad.set_property("zorder", &1u32).unwrap();
        let (width, height) = resolution;
        background_pad.set_property("height", &(height as i32)).unwrap();
        background_pad.set_property("width", &(width as i32)).unwrap();

        let in_pad = background.get_static_pad("src").unwrap();
        in_pad.link(&background_pad);
        
        WmState { resolution, layout: HashMap::new(), widgets: HashMap::new(),
                  pipe, background, background_pad, mixer }
    }

    pub fn plug (&mut self, pad: &SrcPad) {
        match pad.typ {
            Type::Video => {
                let uid;
                let widg = widget_factory::make("video").unwrap();
                {
                    let mut widg = widg.lock().unwrap();
                    widg.add_to_pipe(self.pipe.clone().upcast());
                    uid = widg.gen_uid();
                    let sink_pad = self.mixer.get_request_pad("sink_%u").unwrap();
                    widg.plug_src(pad);
                    widg.plug_sink(sink_pad);
                }
                self.widgets.insert(uid, widg);
            }
            _ => ()
        }
    }
    
    pub fn from_template (&mut self, t: &WmTemplate) -> Result<(),String> {
        Ok(())
    }

    pub fn to_template (&self) -> WmTemplate {
        let resolution = self.resolution;
        let layout     = self.layout.iter()
            .map(|(cname,c)| {
                let position = c.position;
                let widgets  = c.widgets.iter()
                    .map(move |(wname,w)| (wname.clone(), w.lock().unwrap().get_desc().clone()))
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

impl Replybox<WmTemplatePartial,()> for Wm {
    fn reply (&self) -> Box<Fn(WmTemplatePartial)->Result<(),String> + Send + Sync> {
        let state = self.state.clone();
        let chat  = self.chat.clone();
        Box::new(move |templ| {
            let widg = state.lock().unwrap().widgets.iter()
                .map(move |(name,w)| (name.clone(), w.lock().unwrap().get_desc().clone()))
                .collect::<HashMap<String,WidgetDesc>> ();
            let temp = WmTemplate::from_partial(templ, widg);
            match temp.validate() {
                Err(e) => Err(e),
                Ok(()) => {
                    let res = state.lock().unwrap().from_template(&temp);
                    if res.is_ok() { // New layout
                        chat.lock().unwrap().talk(&temp);
                    };
                    res
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
    }

    pub fn src_pad (&self) -> gst::Pad {
        self.state.lock().unwrap().mixer.get_static_pad("src").unwrap()
    }

    pub fn plug (&mut self, pad: &SrcPad) {
        self.state.lock().unwrap().plug(&pad);
        self.chat.lock().unwrap().talk(&self.state.lock().unwrap().to_template());
    }
}
