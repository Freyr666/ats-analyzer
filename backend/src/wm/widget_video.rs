use gst;
use gst::prelude::*;
use std::str::FromStr;
use pad::SrcPad;
use wm::position::Position;
use wm::widget::{Widget,WidgetDesc};

pub struct WidgetVideo {
    desc:      WidgetDesc,
    enabled:   bool,
    uid:       Option<String>,
    stream:    u32,
    channel:   u32,
    pid:       u32,
    mixer_pad: Option<gst::Pad>,
    input_pad: Option<gst::Pad>,
    valve:     gst::Element,
    scale:     gst::Element,
    caps:      gst::Element,
}

impl WidgetVideo {
    pub fn new() -> WidgetVideo {
        let desc = WidgetDesc {
            position: Position::new(),
            typ: String::from("video"),
            aspect: (0,0),
            description: String::from("video widget"),
            layer: 0,
        };
        let valve = gst::ElementFactory::make("valve", None).unwrap();
        let scale = gst::ElementFactory::make("videoscale", None).unwrap();
        let caps  = gst::ElementFactory::make("capsfilter", None).unwrap();
        caps.set_property("caps", &gst::Caps::from_str("video/x-raw,pixel-aspect-ratio=1/1").unwrap()).unwrap();
        WidgetVideo {
            desc,
            enabled:   false,
            uid:       None,
            stream: 0, channel: 0, pid: 0,
            mixer_pad: None, input_pad: None,
            valve, scale, caps,
        }
    }

    fn set_layer(&mut self, layer: i32) {
        if self.desc.layer == layer { return };
        self.desc.layer = layer;
        if let Some(ref pad) = self.mixer_pad {
            pad.set_property("zorder", &(layer+1)).unwrap();
        };
    }

    fn set_position(&mut self, position: Position) {
        if self.desc.position == position { return };
        self.desc.position = position;
        if let Some(ref pad) = self.mixer_pad {
            let cps = format!("video/x-raw,pixel-aspect-ratio=1/1,height={},width={}",
                              position.get_height(), position.get_width());
            self.caps.set_property("caps", &gst::Caps::from_string(&cps).unwrap()).unwrap();
            pad.set_property("height", &position.get_height()).unwrap();
            pad.set_property("wodth", &position.get_width()).unwrap();
            pad.set_property("xpos", &position.get_x()).unwrap();
            pad.set_property("ypos", &position.get_y()).unwrap();
        };
    }
}

impl Widget for WidgetVideo {
    fn add_to_pipe(&self, pipe: gst::Bin) {
        pipe.add_many(&[&self.valve, &self.scale, &self.caps]).unwrap();
        gst::Element::link_many(&[&self.valve, &self.scale, &self.caps]).unwrap();
        self.valve.sync_state_with_parent().unwrap();
        self.scale.sync_state_with_parent().unwrap();
        self.caps.sync_state_with_parent().unwrap();
    }
    
    fn plug_src(&mut self, src: &SrcPad) {
        if self.input_pad.is_some() { return };
        self.stream  = src.stream;
        self.channel = src.channel;
        self.pid     = src.pid;
        let in_pad   = self.valve.get_static_pad("sink").unwrap();
        self.input_pad = Some(in_pad.clone());
        in_pad.connect_property_caps_notify(move |_| {
            
        });
        src.pad.link(&in_pad.clone());
    }
    
    fn plug_sink (&mut self, sink: gst::Pad) {
        self.caps.get_static_pad("src").unwrap().link(&sink);
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
            let s = format!("Vid_{}_{}", self.stream, self.pid);
            self.uid = Some(s.clone());
            s
        }
    }
    
    fn get_desc(&self) -> &WidgetDesc {
        &self.desc
    }
    
    fn apply_desc(&mut self, d: &WidgetDesc) {
        self.set_layer(d.layer);
        self.set_position(d.position);
    }
}

