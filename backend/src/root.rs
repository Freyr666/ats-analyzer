use gst::prelude::*;
use gst;
use std::sync::{Arc,Mutex,Weak};
use std::sync::mpsc::Sender;
use metadata::{Channel,Structure};
use settings::Settings;
use signals::Signal;
use pad::SrcPad;
use branch::{Typ,Branch};
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
                     bin: &gst::Pipeline,
                     pad: &gst::Pad,
                     sender_data: Weak<Mutex<Sender<(Typ,String,u32,u32,gst::Buffer)>>>,
                     sender_status: Weak<Mutex<Sender<(String,u32,u32,bool)>>>) {
        let pname = pad.get_name();
        let pcaps = String::from(pad.get_current_caps().unwrap().get_structure(0).unwrap().get_name());
        let name_toks: Vec<&str> = pname.split('_').collect();
        let caps_toks: Vec<&str> = pcaps.split('/').collect();
        
        if name_toks.len() != 3 || caps_toks.is_empty() { return };
        let typ = caps_toks[0];
        let pid = u32::from_str_radix(name_toks[2], 16).unwrap();
        
        /* No corresponding pid in config */
        if let None = chan.find_pid(pid){
            return;
        };

        debug!("Root::build_branch [{}]", pid);

        if let Some(branch) = Branch::new(stream,
                                          chan.number,
                                          pid,
                                          typ,
                                          settings,
                                          sender_data,
                                          sender_status) {
            branch.add_to_pipe(&bin);
            branch.plug(&pad);
            match branch {
                Branch::Video(ref b) => b.pad_added.lock().unwrap().connect(move |p| {
                    added.lock().unwrap().emit(p)
                }),
                Branch::Audio(ref b) => {
                    b.pad_added.lock().unwrap().connect(move |p| {
                        added.lock().unwrap().emit(p)
                    });
                    b.audio_pad_added.lock().unwrap().connect(move |p| {
                        audio_added.lock().unwrap().emit(p)
                    });
                },
            };

            debug!("Root::build_branch [{} ready]", pid);
            branches.push(branch);
        }
    }
    
    pub fn new(bin: &gst::Pipeline,
               m: &Structure,
               settings: Option<Settings>,
               sender_data: &Arc<Mutex<Sender<(Typ,String,u32,u32,gst::Buffer)>>>,
               sender_status: &Arc<Mutex<Sender<(String,u32,u32,bool)>>>)
               -> Option<Root> {
        debug!("Root::new");

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

        for chan in &m.channels {
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
            
            let stream   = id.clone();
            let chan     = chan.clone();
            let bin_weak = bin.downgrade();
            let settings_c = settings.clone();
            let branches_weak = branches.clone();
            let pad_added_c = pad_added.clone();
            let audio_pad_added_c = audio_pad_added.clone();
            
            let sender_data = Arc::downgrade(sender_data);
            let sender_status = Arc::downgrade(sender_status);
            
            demux.connect_pad_added(move | _, pad | {
                let bin      = bin_weak.clone().upgrade().unwrap();
                //let branches = branches_weak.upgrade().unwrap();
                let settings = settings_c.lock().unwrap();
                Root::build_branch(&mut branches_weak.lock().unwrap(),
                                   stream.clone(), &chan,
                                   *settings,
                                   pad_added_c.clone(), audio_pad_added_c.clone(),
                                   &bin,
                                   pad,
                                   sender_data.clone(),
                                   sender_status.clone());
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
