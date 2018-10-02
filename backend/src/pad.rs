use gst;
use gst::prelude::*;

#[derive(Clone,Copy)]
pub enum Type {
    Video, Audio, Unknown //GraphVolume,
}

pub struct SrcPad {
    pub typ: Type,
    pub stream: String,
    pub channel: u32,
    pub pid: u32,

    bin: gst::Bin, // TODO remove
    tee: gst::Element,
    pub pad: gst::Pad,
}

impl SrcPad {
    pub fn new(stream: String, channel: u32, pid: u32, typ: &str, bin: gst::Bin, pad: &gst::Pad) -> SrcPad {
        let typ = match typ {
            "video" => Type::Video,
            "audio" => Type::Audio,
            _       => Type::Unknown,
        };

        let tee = gst::ElementFactory::make("tee", None).unwrap();
        bin.add(&tee).unwrap();
        tee.sync_state_with_parent().unwrap();

        let tee_sink = tee.get_static_pad("sink").unwrap();
        // TODO check
        let _ = pad.link(&tee_sink);

        let mid_pad = tee.get_request_pad("src_%u").unwrap();
        let ghost   = gst::GhostPad::new(None, &mid_pad).unwrap();
        ghost.set_active(true).unwrap();

        bin.add_pad(&ghost).unwrap();

        SrcPad { typ, stream, channel, pid, bin, tee, pad: ghost.upcast() }
    }
    
}

impl Clone for SrcPad {
    fn clone (&self) -> SrcPad {
        let mid_pad = self.tee.get_request_pad("src_%u").unwrap();
        let ghost   = gst::GhostPad::new(None, &mid_pad).unwrap();
        ghost.set_active(true).unwrap();

        SrcPad { typ: self.typ, stream: self.stream.clone(), channel: self.channel, pid: self.pid,
                 bin: self.bin.clone(), tee: self.tee.clone(), pad: ghost.upcast() }
    }
}

impl Drop for SrcPad {
    fn drop (&mut self) {
        self.bin.remove_pad(&self.pad).unwrap();
        if let Ok(p) = self.pad.clone().downcast::<gst::GhostPad>() {
            self.tee.remove_pad(&p.get_target().unwrap()).unwrap();
        } else {
            self.tee.remove_pad(&self.pad).unwrap();
        }
    }
}
