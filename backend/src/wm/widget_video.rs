use gst;
use gst_video::VideoInfo;
use gst::prelude::*;
use std::str::FromStr;
use pad::{Type,SrcPad};
use signals::Signal;
use std::sync::{Arc,Mutex};
use wm::position::Position;
use wm::widget::{Widget,WidgetDesc,Domain};

pub struct WidgetVideo {
    desc:      Arc<Mutex<WidgetDesc>>,
    par:       Arc<Mutex<Option<(u32,u32)>>>,
    enabled:   bool,
    offset:    (u32, u32),
    uid:       Option<String>,
    stream:    String,
    channel:   u32,
    pid:       u32,
    mixer_pad: Option<gst::Pad>,
    input_pad: Option<gst::Pad>,
    valve:     gst::Element,
    conv:      gst::Element,
    deint:     gst::Element,
    caps:      gst::Element,
    linked:    Arc<Mutex<Signal<()>>>,
}

struct Asp {
    asp: (u32, u32),
    par: (u32, u32),
}

impl WidgetVideo {
    pub fn new() -> WidgetVideo {
        let desc = WidgetDesc {
            position: None,
            typ: Type::Video,
            domain: Domain::Nihil,
            pid: None,
            aspect: None,
            description: String::from("video widget"),
            layer: 0,
        };
        let offset = (0, 0);
        let desc   = Arc::new(Mutex::new(desc));
        let par    = Arc::new(Mutex::new(None));
        let linked = Arc::new(Mutex::new(Signal::new()));
        let valve  = gst::ElementFactory::make("valve", None).unwrap();
        let conv   = gst::ElementFactory::make("glcolorconvert", None).unwrap();
        let deint = gst::ElementFactory::make("gldeinterlace", None).unwrap();
        let caps  = gst::ElementFactory::make("capsfilter", None).unwrap();
        caps.set_property("caps", &gst::Caps::from_str("video/x-raw(ANY),format=RGBA").unwrap()).unwrap();
        WidgetVideo {
            desc, par,
            offset,
            enabled:   false,
            uid:       None,
            stream: String::default(), channel: 0, pid: 0,
            mixer_pad: None, input_pad: None,
            valve, conv, deint, caps,
            linked,
        }
    }

    fn gcd(x: u32, y: u32) -> u32 {
        let mut x = x;
        let mut y = y;
        while y != 0 {
            let t = y;
            y = x % y;
            x = t;
        }
        x
    }

    fn retrieve_aspect (pad: &gst::Pad) -> Option<Asp> {
        if let Some(ref caps) = pad.get_current_caps() {
            let vi = VideoInfo::from_caps(caps).unwrap();
            let (width, height) = (vi.width(), vi.height());
            let (par_n, par_d)  = vi.par().into();
            let x = width * (par_n as u32);
            let y = height * (par_d as u32);
            let gcd = WidgetVideo::gcd(x, y);
            Some(Asp { asp: (x/gcd, y/gcd), par: (par_n as u32, par_d as u32) })
        } else {
            None
        }
    }

    fn enable (&mut self) {
        if self.enabled { return };
        self.enabled = true;
        if let Some(ref pad) = self.mixer_pad {
            pad.set_property("alpha", &1.0).unwrap();
        };
        self.valve.set_property("drop", &false).unwrap();
    }
}

impl Widget for WidgetVideo {
    fn add_to_pipe(&self, pipe: &gst::Bin) {
        pipe.add_many(&[&self.valve, &self.conv, /*&self.scale,*/ &self.deint, &self.caps/*, &self.queue*/]).unwrap();
        gst::Element::link_many(&[&self.valve, &self.conv, /*&self.scale,*/ &self.deint, &self.caps/*, &self.queue*/]).unwrap();
        self.valve.sync_state_with_parent().unwrap();
        //self.scale.sync_state_with_parent().unwrap();
        self.deint.sync_state_with_parent().unwrap();
        self.conv.sync_state_with_parent().unwrap();
        //self.queue.sync_state_with_parent().unwrap();
        self.caps.sync_state_with_parent().unwrap();
    }
    
    fn plug_src(&mut self, src: &SrcPad) {
        if self.input_pad.is_some() { return };
        self.stream  = src.stream.clone();
        self.channel = src.channel;
        self.pid     = src.pid;
        let in_pad   = self.valve.get_static_pad("sink").unwrap();
        self.input_pad = Some(in_pad.clone());
        
        self.desc.lock().unwrap().domain =
            Domain::Chan { stream: src.stream.clone(), channel: src.channel };
        self.desc.lock().unwrap().pid = Some(src.pid);

        let desc   = self.desc.clone();
        let par    = self.par.clone();
        let linked = self.linked.clone();
        in_pad.connect_property_caps_notify(move |pad| {
            if let Some (aspect) = WidgetVideo::retrieve_aspect(pad) {
                desc.lock().unwrap().aspect = Some(aspect.asp); // TODO fix that
                *par.lock().unwrap() = Some(aspect.par);
                linked.lock().unwrap().emit(&());
            }
        });
        let _ = src.pad.link(&in_pad.clone()); // TODO
    }
    
    fn plug_sink (&mut self, sink: gst::Pad) {
        let _ = self.caps.get_static_pad("src").unwrap().link(&sink); // TODO
        if ! self.enabled {
            self.valve.set_property("drop", &true).unwrap();
            sink.set_property("alpha", &0.0).unwrap();
        }
        self.mixer_pad = Some(sink.clone());
    }

    fn disable (&mut self) {
        if !self.enabled { return };
        let mut desc = self.desc.lock().unwrap();
        self.enabled = false;
        if let Some(ref pad) = self.mixer_pad {
            pad.set_property("alpha", &0.0).unwrap();
        };
        self.valve.set_property("drop", &true).unwrap();
        desc.position = None;
    }

    fn render (&mut self, container_position: Position, position: Position, layer: i32) {
        if ! self.enabled { self.enable (); }

        let mut desc     = self.desc.lock().unwrap();

        let offset    = (container_position.get_x(), container_position.get_y());
        desc.position = Some(position);
        desc.layer    = layer;
        self.offset   = offset;

        let resolution = (container_position.w, container_position.h);
        let pos        = position.to_absolute(resolution);
        let (off_x, off_y) = offset;

        // Non-square-pixel-related hack
        let (height, width) : (i32, i32) = if let Some(par) = *self.par.lock().unwrap() {
            let (par_n, par_d) = par;
            (pos.get_height() as i32, (pos.get_width() * par_d / par_n) as i32)
        } else {
            (pos.get_height() as i32, pos.get_width() as i32)
        };

        if let Some(ref pad) = self.mixer_pad {
            pad.set_property("zorder", &((layer+1) as u32)).unwrap();
            pad.set_property("height", &height).unwrap();
            pad.set_property("width", &width).unwrap();
            pad.set_property("xpos", &((pos.get_x() + off_x) as i32)).unwrap();
            pad.set_property("ypos", &((pos.get_y() + off_y) as i32)).unwrap();
        };
    }
    
    fn gen_uid(&mut self) -> String {
        if let Some(ref s) = self.uid {
            s.clone()
        } else {
            let s = format!("Vid_{}_{}", self.stream, self.pid);
            self.uid = Some(s.clone()); // TODO consider stream =/= 0 check
            s
        }
    }
    
    fn get_desc(&self) -> WidgetDesc {
        self.desc.lock().unwrap().clone()
    }
    
    fn linked(&self) -> Arc<Mutex<Signal<()>>> {
        self.linked.clone()
    }
}
