use gst;
use gst::prelude::*;
use std::str::FromStr;
use std::sync::{Arc,Mutex};
use signals::Signal;
use pad::{Type,SrcPad};
use wm::position::Position;
use wm::widget::{Widget,WidgetDesc,Domain};

pub struct WidgetSoundbar {
    desc:      Arc<Mutex<WidgetDesc>>,
    enabled:   bool,
    uid:       Option<String>,
    stream:    String,
    channel:   u32,
    pid:       u32,
    mixer_pad: Option<gst::Pad>,
    input_pad: Option<gst::Pad>,
    valve:     gst::Element,
    soundbar:  gst::Element,
    upload:    gst::Element,
    caps:      gst::Element,
    linked:    Arc<Mutex<Signal<()>>>,
}

impl WidgetSoundbar {
    pub fn new() -> WidgetSoundbar {
        let desc = WidgetDesc {
            position: Position::new(),
            typ: Type::Audio,
            domain: Domain::Nihil,
            pid: None,
            aspect: None,
            description: String::from("soundbar widget"),
            layer: 0,
        };
        let desc     = Arc::new(Mutex::new(desc));
        let linked   = Arc::new(Mutex::new(Signal::new()));
        let soundbar = gst::ElementFactory::make("soundbar", None).unwrap();
        let upload   = gst::ElementFactory::make("glupload", None).unwrap();
        let caps     = gst::ElementFactory::make("capsfilter", None).unwrap();
        let valve    = gst::ElementFactory::make("valve", None).unwrap();
        caps.set_property("caps", &gst::Caps::from_str("video/x-raw").unwrap()).unwrap();
        WidgetSoundbar {
            desc,
            enabled:   false,
            uid:       None,
            stream: String::default(), channel: 0, pid: 0,
            mixer_pad: None, input_pad: None,
            valve, soundbar, upload, caps, linked,
        }
    }

    fn set_layer(&mut self, layer: i32) {
        let mut desc = self.desc.lock().unwrap();
        if desc.layer == layer { return };
        desc.layer = layer;
        if let Some(ref pad) = self.mixer_pad {
            pad.set_property("zorder", &((layer+1) as u32)).unwrap();
        };
    }

    fn set_position(&mut self, position: Position) {
        let mut desc = self.desc.lock().unwrap();
        if desc.position == position { return };
        desc.position = position;
        if let Some(ref pad) = self.mixer_pad {
            let cps = format!("video/x-raw,height={},width={}",
                              position.get_height(), position.get_width());
            self.caps.set_property("caps", &gst::Caps::from_string(&cps).unwrap()).unwrap();
            pad.set_property("height", &(position.get_height() as i32)).unwrap();
            pad.set_property("width", &(position.get_width() as i32)).unwrap();
            pad.set_property("xpos", &(position.get_x() as i32)).unwrap();
            pad.set_property("ypos", &(position.get_y() as i32)).unwrap();
        };
    }
}

impl Widget for WidgetSoundbar {
    fn add_to_pipe (&self, pipe: &gst::Bin) {
        // TODO fix valve after soundbar
        pipe.add_many(&[&self.soundbar, &self.valve, &self.caps, &self.upload]).unwrap();
        gst::Element::link_many(&[&self.soundbar, &self.valve, &self.caps, &self.upload]).unwrap();
        self.soundbar.sync_state_with_parent().unwrap();
        self.upload.sync_state_with_parent().unwrap();
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
        let _ = self.upload.get_static_pad("src").unwrap().link(&sink);
        if ! self.enabled {
            self.valve.set_property("drop", &true).unwrap();
            sink.set_property("alpha", &0.0).unwrap();
        }
        self.mixer_pad = Some(sink.clone());
    }

    fn set_enable (&mut self, enabled: bool) {
        if self.enabled == enabled { return };
        self.enabled = enabled;
        if let Some(ref pad) = self.mixer_pad {
            if enabled {
                pad.set_property("alpha", &1.0).unwrap();
            } else {
                pad.set_property("alpha", &0.0).unwrap();
            }
        };
        if enabled {
            self.valve.set_property("drop", &false).unwrap();
        } else {
            self.valve.set_property("drop", &true).unwrap();
        }
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

    fn apply_desc(&mut self, d: &WidgetDesc) {
        self.set_layer(d.layer);
        self.set_position(d.position);
    }

    fn linked(&self) -> Arc<Mutex<Signal<()>>> {
        self.linked.clone()
    }
}
