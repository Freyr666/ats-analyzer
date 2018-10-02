use gst::prelude::*;
use gst;
use std::sync::{Arc,Mutex};
use std::sync::mpsc::Sender;
use metadata::{Channel,Structure};
use settings::Settings;
use signals::Signal;
use chatterer::MsgType;
use pad::SrcPad;
use branch::Branch;
use std::u32;

pub struct Root {
    settings:      Arc<Mutex<Option<Settings>>>,
    branches:      Arc<Mutex<Vec<Branch>>>,
    pub pad_added: Arc<Mutex<Signal<SrcPad>>>,
    pub audio_pad_added: Arc<Mutex<Signal<SrcPad>>>,
}

impl Root {

    fn build_branch (branches: &mut Vec<Branch>,
                     stream: String, chan: &Channel,
                     settings: Option<Settings>,
                     added: Arc<Mutex<Signal<SrcPad>>>,
                     audio_added: Arc<Mutex<Signal<SrcPad>>>,
                     bin: &gst::Bin, pad: &gst::Pad,
                     format: MsgType, sender: &Mutex<Sender<Vec<u8>>>) {
        let pname = pad.get_name();
        let pcaps = String::from(pad.get_current_caps().unwrap().get_structure(0).unwrap().get_name());
        let name_toks: Vec<&str> = pname.split('_').collect();
        let caps_toks: Vec<&str> = pcaps.split('/').collect();
        
        if name_toks.len() != 3 || caps_toks.is_empty() { return };
        let typ = caps_toks[0];
        let pid = u32::from_str_radix(name_toks[2], 16).unwrap();
        if let Some (p) = chan.find_pid(pid){
            if !p.to_be_analyzed { return };
        };

        if let Some(branch) = Branch::new(stream, chan.number, pid, typ, settings, format, sender.lock().unwrap().clone()) {
            branch.add_to_pipe(&bin);
            branch.plug(&pad);
            match branch {
                Branch::Video(ref b) => b.pad_added.lock().unwrap().connect(move |p| added.lock().unwrap().emit(p)),
                Branch::Audio(ref b) => {
                    b.pad_added.lock().unwrap().connect(move |p| added.lock().unwrap().emit(p));
                    b.audio_pad_added.lock().unwrap().connect(move |p| audio_added.lock().unwrap().emit(p));
                },
            };

            branches.push(branch);
        }
    }
    
    pub fn new(bin: &gst::Bin, m: Structure, settings: Option<Settings>,
               format: MsgType, sender: Sender<Vec<u8>>) -> Option<Root> {
        if ! m.to_be_analyzed() { return None };

        let src             = gst::ElementFactory::make("udpsrc", None).unwrap();
        let tee             = gst::ElementFactory::make("tee", None).unwrap();
        let settings        = Arc::new(Mutex::new(settings));
        let branches        = Arc::new(Mutex::new(Vec::new()));
        let pad_added       = Arc::new(Mutex::new(Signal::new()));
        let audio_pad_added = Arc::new(Mutex::new(Signal::new()));
        
        src.set_property("uri", &m.uri).unwrap();
        src.set_property("buffer-size", &2_147_483_647).unwrap();

        bin.add_many(&[&src, &tee]).unwrap();
        src.link(&tee).unwrap();

        let id = m.id.clone();

        for chan in m.channels {
            let demux_name = format!("demux_{}_{}_{}_{}",
                                     id.clone(), chan.number, chan.service_name, chan.provider_name);

            let queue = gst::ElementFactory::make("queue", None).unwrap();
            let demux = gst::ElementFactory::make("tsdemux", Some(demux_name.as_str())).unwrap();

            demux.set_property("program-number", &(chan.number as i32)).unwrap();
            queue.set_property("max-size-time", &0u64).unwrap();
            queue.set_property("max-size-buffers", &0u32).unwrap();
            queue.set_property("max-size-bytes", &0u32).unwrap();

            let sinkpad = queue.get_static_pad("sink").unwrap();
            let srcpad  = tee.get_request_pad("src_%u").unwrap();

            bin.add_many(&[&queue,&demux]).unwrap();
            queue.link(&demux).unwrap();

            // TODO check
            let _ = srcpad.link(&sinkpad);
            
            let stream = id.clone();
            let bin_cc = bin.clone();
            let settings_c = settings.clone();
            let branches_c = branches.clone();
            let pad_added_c = pad_added.clone();
            let audio_pad_added_c = audio_pad_added.clone();
            let sender_c = Mutex::new(sender.clone());
            
            demux.connect_pad_added(move | _, pad | {
                let settings = settings_c.lock().unwrap();
                Root::build_branch(&mut branches_c.lock().unwrap(), stream.clone(), &chan,
                                   *settings,
                                   pad_added_c.clone(), audio_pad_added_c.clone(),
                                   &bin_cc, pad,
                                   format, &sender_c);
            });
        };

        Some(Root { settings, branches, pad_added, audio_pad_added })
    }

    pub fn apply_settings(&mut self, s: Settings) {
        self.branches.lock()
            .unwrap()
            .iter_mut()
            .for_each(|b : &mut Branch| b.apply_settings(s));
        *self.settings.lock().unwrap() = Some(s);
    }
}
