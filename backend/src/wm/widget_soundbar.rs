use gst;
use gst::prelude::*;
use std::str::FromStr;
use std::sync::{Arc,Mutex};
use signals::Signal;
use pad::{Type,SrcPad};
use wm::position::Position;
use wm::position::Absolute;
use wm::widget::{Widget,WidgetDesc,Domain};

pub struct WidgetSoundbar {
    desc:      Arc<Mutex<WidgetDesc>>,
    enabled:   bool,
    offset:    (u32, u32),
    uid:       Option<String>,
    stream:    String,
    channel:   u32,
    pid:       u32,
    mixer_pad: Option<gst::Pad>,
    input_pad: Option<gst::Pad>,
    valve:     gst::Element,
    soundbar:  gst::Element,
    //upload:    gst::Element,
    caps:      gst::Element,
    linked:    Arc<Mutex<Signal<()>>>,
}

impl WidgetSoundbar {
    pub fn new() -> WidgetSoundbar {
        let desc = WidgetDesc {
            position: None,
            typ: Type::Audio,
            domain: Domain::Nihil,
            pid: None,
            aspect: None,
            description: String::from("soundbar widget"),
            layer: 0,
        };
        let offset   = (0, 0);
        let desc     = Arc::new(Mutex::new(desc));
        let linked   = Arc::new(Mutex::new(Signal::new()));
        let soundbar = gst::ElementFactory::make("glsoundbar", None).unwrap();
        //let upload   = gst::ElementFactory::make("glupload", None).unwrap();
        let caps     = gst::ElementFactory::make("capsfilter", None).unwrap();
        let valve    = gst::ElementFactory::make("valve", None).unwrap();
        caps.set_property("caps", &gst::Caps::from_str("video/x-raw(ANY)").unwrap()).unwrap();
        WidgetSoundbar {
            desc,
            offset,
            enabled:   false,
            uid:       None,
            stream: String::default(), channel: 0, pid: 0,
            mixer_pad: None, input_pad: None,
            valve, soundbar,/* upload,*/ caps, linked,
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

impl Widget for WidgetSoundbar {
    fn add_to_pipe (&self, pipe: &gst::Bin) {
        // TODO fix valve after soundbar
        pipe.add_many(&[&self.soundbar, &self.valve, &self.caps/*, &self.upload*/]).unwrap();
        gst::Element::link_many(&[&self.soundbar, &self.valve, &self.caps/*, &self.upload*/]).unwrap();
        self.soundbar.sync_state_with_parent().unwrap();
        //self.upload.sync_state_with_parent().unwrap();
        self.valve.sync_state_with_parent().unwrap();
        self.caps.sync_state_with_parent().unwrap();
    }

    fn plug_src (&mut self, src: &SrcPad) {
        if self.input_pad.is_some() { return }; //is plugged already
        self.stream  = src.stream.clone();
        self.channel = src.channel;
        self.pid     = src.pid;
        let in_pad   = self.soundbar.get_static_pad("sink").unwrap();
        self.input_pad = Some(in_pad.clone());

        self.desc.lock().unwrap().domain =
            Domain::Chan { stream: src.stream.clone(), channel: src.channel };
        self.desc.lock().unwrap().pid = Some(src.pid);

        // TODO check
        let _ = src.pad.link(&in_pad.clone());
        self.linked.lock().unwrap().emit(&());
    }

    fn plug_sink (&mut self, sink: gst::Pad) {
        // TODO check
        let _ = self.caps.get_static_pad("src").unwrap().link(&sink);
        if ! self.enabled {
            self.valve.set_property("drop", &true).unwrap();
            sink.set_property("alpha", &0.0).unwrap();
        }
        self.mixer_pad = Some(sink.clone());
    }

    fn disable (&mut self) {
        if ! self.enabled { return };
        let mut desc = self.desc.lock().unwrap();
        self.enabled = false;
        if let Some(ref pad) = self.mixer_pad {
            pad.set_property("alpha", &0.0).unwrap();
        };
        self.valve.set_property("drop", &true).unwrap();
        desc.position = None;
    }

    fn render (&mut self, container: &Absolute, position: Position, layer: i32) {
        if ! self.enabled { self.enable (); }

        let mut desc  = self.desc.lock().unwrap();
        let (off_x, off_y) = (container.left, container.top);
        let resolution = (container.width, container.height);

        desc.position = Some(position);
        desc.layer    = layer;
        self.offset   = (off_x, off_y);

        let Absolute{left: xpos, top: ypos, width, height} =
            position.denormalize(resolution);

        if let Some(ref pad) = self.mixer_pad {
            let cps = format!("video/x-raw(ANY),height={},width={}", height, width);
            self.caps.set_property("caps", &gst::Caps::from_str(&cps).unwrap()).unwrap();
            pad.set_property("zorder", &((layer+1) as u32)).unwrap();
            pad.set_property("height", &(height as i32)).unwrap();
            pad.set_property("width", &(width as i32)).unwrap();
            pad.set_property("xpos", &((xpos + off_x) as i32)).unwrap();
            pad.set_property("ypos", &((ypos + off_y) as i32)).unwrap();
        };
    }

    fn gen_uid(&mut self) -> String {
        if let Some(ref s) = self.uid {
            s.clone()
        } else {
            let s = format!("Sbar_{}_{}", self.stream, self.pid);
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
