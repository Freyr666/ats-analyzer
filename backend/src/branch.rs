use std::sync::{Arc,Mutex};
use std::sync::mpsc::Sender;
use gst::prelude::*;
use gst;
use pad::SrcPad;
use settings::Settings;
use video_data::VideoData;
use audio_data::AudioData;
use signals::Signal;

struct CommonBranch {
    decoder: gst::Element,
    bin: gst::Bin,
    sink: gst::Pad,
}

impl CommonBranch {
    pub fn new () -> CommonBranch {
        let bin   = gst::Bin::new(None);

        for el in &["vaapih264dec", "vaapimpeg2dec", "vaapidecodebin"] {
            if let Some(f) = gst::ElementFactory::find(&el) {
                f.set_rank(0)
            }
        };

        let decoder = gst::ElementFactory::make("decodebin",None).unwrap();
        
        bin.add_many(&[/*&queue,*/ &decoder]).unwrap();
        decoder.sync_state_with_parent().unwrap();
        
        let p = decoder.get_static_pad("sink").unwrap();
        let sink_ghost = gst::GhostPad::new(Some("sink"), &p).unwrap();

        sink_ghost.set_active(true).unwrap();
        bin.add_pad(&sink_ghost).unwrap();

        CommonBranch { decoder, bin, sink: sink_ghost.upcast() }
    }
}

#[derive(Clone)]
pub struct VideoBranch {
    stream:   String,
    channel:  u32,
    pid:      u32,
    pads:     Arc<Mutex<Vec<SrcPad>>>,
    analyser: gst::Element,
    decoder:  gst::Element,
    bin:      gst::Bin,
    sink:     gst::Pad,
    
    pub pad_added: Arc<Mutex<Signal<SrcPad>>>,
}

//fn enum_to_val(cls: &str, val: i32) -> glib::Value {
//    glib::EnumClass::new(glib::Type::from_name(cls).unwrap()).unwrap().to_value(val).unwrap()
//}

impl VideoBranch {
    pub fn new (stream: String, channel: u32, pid: u32,
                settings: Option<Settings>, sender: Sender<Vec<u8>>) -> VideoBranch {
        debug!("VideoBranch::create");
        
        let common = CommonBranch::new();

        let pads      = Arc::new(Mutex::new(Vec::new()));
        let analyser  = gst::ElementFactory::make("videoanalysis",None).unwrap(); //videoanalysis
        let decoder   = common.decoder;
        let bin       = common.bin;
        let sink      = common.sink;
        let pad_added = Arc::new(Mutex::new(Signal::new()));

        analyser.set_property("latency", &3u32).unwrap();
        
        VideoBranch::apply_settings(&analyser, settings);
        // TODO consider lock removal
        let vdata     = Arc::new(Mutex::new(VideoData::new(stream.clone(), channel, pid, sender)));        

        let stream_id     = stream.clone();
        let bin_weak      = bin.downgrade();
        let analyser_weak = analyser.downgrade();
        let pads_c        = pads.clone();
        let pad_added_c   = pad_added.clone();
        
        decoder.connect_pad_added(move | _, pad | {
            debug!("VideoBranch::create encoder initialized");
            let pcaps = String::from(pad.get_current_caps().unwrap().get_structure(0).unwrap().get_name());
            let caps_toks: Vec<&str> = pcaps.split('/').collect();
            if caps_toks.is_empty() { return };

            let typ = &caps_toks[0];

            if *typ != "video" { return };

            let bin = bin_weak.upgrade().unwrap();
            let analyser = analyser_weak.upgrade().unwrap();

            let queue  = gst::ElementFactory::make("queue", None).unwrap();
            let upload = gst::ElementFactory::make("glupload",None).unwrap();

            let sink_pad = queue.get_static_pad("sink").unwrap();
            let src_pad  = analyser.get_static_pad("src").unwrap();

            queue.set_property("max-size-time", &0u64).unwrap();
            queue.set_property("max-size-buffers", &0u32).unwrap();
            queue.set_property("max-size-bytes", &0u32).unwrap();

            bin.add_many(&[&queue, &upload, &analyser]).unwrap();
            gst::Element::link_many(&[&queue, &upload, &analyser]).unwrap();

            queue.sync_state_with_parent().unwrap();
            upload.sync_state_with_parent().unwrap();
            analyser.sync_state_with_parent().unwrap();

            let vdata       = Arc::downgrade(&vdata);
            let vdata_lost  = vdata.clone();
            let vdata_found = vdata.clone();
            // TODO add err check
            analyser.connect("data", true, move |vals| {
                let vdata = vdata.upgrade().unwrap();
                let d: gst::Buffer = vals[1].get::<gst::Buffer>().expect("Expect d");
                vdata.lock().unwrap().send_msg(&d);
                None
            }).unwrap();

            analyser.connect("stream-lost", true, move |_| {
                let vdata_lost = vdata_lost.upgrade().unwrap();
                vdata_lost.lock().unwrap().send_lost();
                debug!("Stream lost sent");
                None
            }).unwrap();

            analyser.connect("stream-found", true, move |_| {
                let vdata_found = vdata_found.upgrade().unwrap();
                vdata_found.lock().unwrap().send_found();
                debug!("Stream found sent");
                None
            }).unwrap();

            let _ = pad.link(&sink_pad); // TODO

            let spad = SrcPad::new(stream_id.clone(), channel, pid, "video", &bin, &src_pad);

            debug!("VideoBranch::create emit pad");
            pad_added_c.lock().unwrap().emit(&spad);
            pads_c.lock().unwrap().push(spad);
        });
        
        VideoBranch { stream, channel, pid, pads,
                      analyser, decoder, bin,
                      sink, pad_added }
    }

    pub fn plug(&self, src_pad: &gst::Pad) {
        let _ = src_pad.link(&self.sink); // TODO
        self.bin.sync_state_with_parent().unwrap();
    }

    pub fn add_to_pipe (&self, b: &gst::Pipeline) {
        b.add(&self.bin).unwrap();
        self.bin.sync_state_with_parent().unwrap();
       // self.bin.sync_children_states().unwrap();
    }

    pub fn apply_settings (analyser: &gst::Element, s: Option<Settings>) {
        if let Some(s) = s {
            let vset = s.video;
            analyser.set_property("loss", &vset.loss).unwrap();
            analyser.set_property("black-pixel-lb", &vset.black.black_pixel).unwrap();
            analyser.set_property("pixel-diff-lb", &vset.freeze.pixel_diff).unwrap();
            analyser.set_property("black-cont", &vset.black.black.cont).unwrap();
            analyser.set_property("black-cont-en", &vset.black.black.cont_en).unwrap();
            analyser.set_property("black-peak", &vset.black.black.peak).unwrap();
            analyser.set_property("black-peak-en", &vset.black.black.peak_en).unwrap();
            analyser.set_property("black-duration", &vset.black.black.duration).unwrap();
            analyser.set_property("luma-cont", &vset.black.luma.cont).unwrap();
            analyser.set_property("luma-cont-en", &vset.black.luma.cont_en).unwrap();
            analyser.set_property("luma-peak", &vset.black.luma.peak).unwrap();
            analyser.set_property("luma-peak-en", &vset.black.luma.peak_en).unwrap();
            analyser.set_property("luma-duration", &vset.black.luma.duration).unwrap();
            analyser.set_property("freeze-cont", &vset.freeze.freeze.cont).unwrap();
            analyser.set_property("freeze-cont-en", &vset.freeze.freeze.cont_en).unwrap();
            analyser.set_property("freeze-peak", &vset.freeze.freeze.peak).unwrap();
            analyser.set_property("freeze-peak-en", &vset.freeze.freeze.peak_en).unwrap();
            analyser.set_property("freeze-duration", &vset.freeze.freeze.duration).unwrap();
            analyser.set_property("diff-cont", &vset.freeze.diff.cont).unwrap();
            analyser.set_property("diff-cont-en", &vset.freeze.diff.cont_en).unwrap();
            analyser.set_property("diff-peak", &vset.freeze.diff.peak).unwrap();
            analyser.set_property("diff-peak-en", &vset.freeze.diff.peak_en).unwrap();
            analyser.set_property("diff-duration", &vset.freeze.diff.duration).unwrap();
            analyser.set_property("blocky-cont", &vset.blocky.blocky.cont).unwrap();
            analyser.set_property("blocky-cont-en", &vset.blocky.blocky.cont_en).unwrap();
            analyser.set_property("blocky-peak", &vset.blocky.blocky.peak).unwrap();
            analyser.set_property("blocky-peak-en", &vset.blocky.blocky.peak_en).unwrap();
            analyser.set_property("blocky-duration", &vset.blocky.blocky.duration).unwrap();
        }
    }
}

#[derive(Clone)]
pub struct AudioBranch {
    stream:   String,
    channel:  u32,
    pid:      u32,
    pads:     Arc<Mutex<Vec<SrcPad>>>,
    analyser: gst::Element,
    decoder:  gst::Element,
    bin:      gst::Bin,
    sink:     gst::Pad,
    
    pub pad_added: Arc<Mutex<Signal<SrcPad>>>,
    pub audio_pad_added: Arc<Mutex<Signal<SrcPad>>>,
}

impl AudioBranch {
    pub fn new (stream: String, channel: u32, pid: u32,
                settings: Option<Settings>, sender: Sender<Vec<u8>>) -> AudioBranch {
        debug!("AudioBranch::create");
        
        let common = CommonBranch::new();

        let pads      = Arc::new(Mutex::new(Vec::new()));
        let analyser  = gst::ElementFactory::make("audioanalysis",None).unwrap();
        let decoder   = common.decoder;
        let bin       = common.bin;
        let sink      = common.sink;
        let pad_added = Arc::new(Mutex::new(Signal::new()));
        let audio_pad_added = Arc::new(Mutex::new(Signal::new()));

        AudioBranch::apply_settings(&analyser, settings);

        // TODO consider lock removal
        let adata = Arc::new(Mutex::new(AudioData::new(stream.clone(), channel, pid, sender)));

        let stream_id = stream.clone();
        let bin_weak      = bin.downgrade();
        let analyser_weak = analyser.downgrade();
        let pads_c = pads.clone();
        let pad_added_c = pad_added.clone();
        let audio_pad_added_c = audio_pad_added.clone();

        // TODO replace gst::pad with Pad
        decoder.connect_pad_added(move | _, pad | {
            debug!("AudioBranch::create encoder initialized");
            let pcaps = String::from(pad.get_current_caps().unwrap().get_structure(0).unwrap().get_name());
            let caps_toks: Vec<&str> = pcaps.split('/').collect();
            if caps_toks.is_empty() { return };

            let typ = &caps_toks[0];

            if *typ != "audio" { return };

            let bin      = bin_weak.upgrade().unwrap();
            let analyser = analyser_weak.upgrade().unwrap();
            
            let queue  = gst::ElementFactory::make("queue", None).unwrap();
            let conv   = gst::ElementFactory::make("audioconvert",None).unwrap();

            let sink_pad = queue.get_static_pad("sink").unwrap();
            let src_pad  = analyser.get_static_pad("src").unwrap();

            queue.set_property("max-size-time", &0u64).unwrap();
            queue.set_property("max-size-buffers", &0u32).unwrap();
            queue.set_property("max-size-bytes", &0u32).unwrap();

            bin.add_many(&[&queue, &conv, &analyser]).unwrap();
            gst::Element::link_many(&[&queue, &conv, &analyser]).unwrap();

            queue.sync_state_with_parent().unwrap();
            conv.sync_state_with_parent().unwrap();
            analyser.sync_state_with_parent().unwrap();

            let adata       = Arc::downgrade(&adata);
            let adata_lost  = adata.clone();
            let adata_found = adata.clone();

            analyser.connect("data", true, move |vals| {
                let adata = adata.upgrade().unwrap();
                let d: gst::Buffer = vals[1].get::<gst::Buffer>().expect("Expect d");
                adata.lock().unwrap().send_msg(&d);
                None
            }).unwrap();

            analyser.connect("stream-lost", true, move |_| {
                let adata_lost = adata_lost.upgrade().unwrap();
                adata_lost.lock().unwrap().send_lost();
                None
            }).unwrap();

            analyser.connect("stream-found", true, move |_| {
                let adata_found = adata_found.upgrade().unwrap();
                adata_found.lock().unwrap().send_found();
                None
            }).unwrap();
            
            let _ = pad.link(&sink_pad); // TODO

            let spad = SrcPad::new(stream_id.clone(), channel, pid, "audio", &bin, &src_pad);
            let aspad = spad.clone();

            debug!("AudioBranch::create emit pad");
            pad_added_c.lock().unwrap().emit(&spad);
            audio_pad_added_c.lock().unwrap().emit(&aspad);
            pads_c.lock().unwrap().push(spad);
            pads_c.lock().unwrap().push(aspad);
        });

        AudioBranch { stream, channel, pid, pads,
                      analyser, decoder, bin, sink,
                      pad_added, audio_pad_added }
    }

    pub fn plug(&self, src_pad: &gst::Pad) {
        let _ = src_pad.link(&self.sink); // TODO
        self.bin.sync_state_with_parent().unwrap();
    }

    pub fn add_to_pipe (&self, b: &gst::Pipeline) {
        b.add(&self.bin).unwrap();
        self.bin.sync_state_with_parent().unwrap();
        self.bin.sync_children_states().unwrap();
    }

    pub fn apply_settings (analyser: &gst::Element, s: Option<Settings>) {
        if let Some(s) = s {
            let aset = s.audio;
            analyser.set_property("loss", &aset.loss).unwrap();
            analyser.set_property("silence-cont", &aset.silence.silence.cont).unwrap();
            analyser.set_property("silence-cont-en", &aset.silence.silence.cont_en).unwrap();
            analyser.set_property("silence-peak", &aset.silence.silence.peak).unwrap();
            analyser.set_property("silence-peak-en", &aset.silence.silence.peak_en).unwrap();
            analyser.set_property("silence-duration", &aset.silence.silence.duration).unwrap();
            analyser.set_property("loudness-cont", &aset.loudness.loudness.cont).unwrap();
            analyser.set_property("loudness-cont-en", &aset.loudness.loudness.cont_en).unwrap();
            analyser.set_property("loudness-peak", &aset.loudness.loudness.peak).unwrap();
            analyser.set_property("loudness-peak-en", &aset.loudness.loudness.peak_en).unwrap();
            analyser.set_property("loudness-duration", &aset.loudness.loudness.duration).unwrap();
            //analyser.set_property("adv-diff", &aset.adv.adv_diff).unwrap();
            //analyser.set_property("adv-buff", &aset.adv.adv_buf).unwrap();
        }
    }

}

#[derive(Clone)]
pub enum Branch {
    Video(VideoBranch),
    Audio(AudioBranch),
}

impl Branch {

    pub fn new(stream: String, channel: u32, pid: u32, typ: &str,
               settings: Option<Settings>, sender: Sender<Vec<u8>>) -> Option<Branch> {
        match typ {
            "video" => Some(Branch::Video(VideoBranch::new(stream, channel, pid, settings, sender))),
            "audio" => Some(Branch::Audio(AudioBranch::new(stream, channel, pid, settings, sender))),
            _       => None
        }
    }
    
    pub fn plug(&self, src_pad: &gst::Pad) {
        match *self {
            Branch::Video(ref br) => br.plug(src_pad),
            Branch::Audio(ref br) => br.plug(src_pad),
        };
    }

    pub fn add_to_pipe (&self, b: &gst::Pipeline) {
        match *self {
            Branch::Video(ref br) => br.add_to_pipe(b),
            Branch::Audio(ref br) => br.add_to_pipe(b),
        }
    }

    pub fn apply_settings (&mut self, s: Settings) {
        match *self {
            Branch::Video(ref br) => VideoBranch::apply_settings(&br.analyser, Some(s)),
            Branch::Audio(ref br) => AudioBranch::apply_settings(&br.analyser, Some(s)),
        }
    }
}
