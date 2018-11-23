/*
use std::collections::HashMap;
use std::vec::Vec;
use std::sync::{Arc,Mutex};
use std::sync::mpsc::Sender;
use chatterer::control::{Addressable,Replybox};
use chatterer::control::message::{Request,Reply};
use chatterer::notif::Notifier;
use gst::prelude::*;
use gst;
use pad::SrcPad;

#[derive(Serialize,Deserialize,Clone)]
struct MuxInfo {
    sources:  Vec<String>,
    chosen:   Option<String>,
}

struct MuxState {
    sources:  HashMap<String,gst::Pad>,
    chosen:   Option<String>,
    selector: gst::Element,
}

pub struct Mux {
    pub chat:    Arc<Mutex<Notifier>>,
    state:       Arc<Mutex<Option<MuxState>>>,
}

impl MuxState {
    fn new (pipe: &gst::Pipeline) -> MuxState {
        let sources  = HashMap::new ();
        let chosen   = None;
        let selector = gst::ElementFactory::make("input-selector", None).unwrap();
        pipe.add_many(&[&selector]).unwrap();
        MuxState { sources, chosen, selector }
    }

    fn info (&self) -> MuxInfo {
        let chosen  = self.chosen.clone ();
        let mut sources = Vec::with_capacity (self.sources.capacity());
        for s in self.sources.keys() {
            sources.push(s.clone());
        };
        MuxInfo { chosen, sources }
    }

    fn apply (&mut self, choice: String) -> Result<(),String> {
        match self.chosen {
            Some(ref chosen) if *chosen == choice => Ok (()),
            _ => {
                if let Some(p) = self.sources.get(&choice) {
                    self.chosen = Some(choice);
                    self.selector.set_property("active-pad", &p).unwrap();
                    Ok(())
                } else {
                    Err(String::from("Pid not found"))
                }
            },
        }
    }

    fn plug (&mut self, pad: &SrcPad) -> bool {
        debug!("AudioMux::plug");
        let name = format!("{}_{}_({})", pad.pid, pad.channel, pad.stream);
        if ! self.sources.contains_key (&name) {
            let sink = self.selector.get_request_pad("sink_%u").unwrap();
            let _ = pad.pad.link(&sink); // TODO 
            // choose the first appeared pad
            if self.chosen.is_none() {
                self.chosen = Some(name.clone());
                self.selector.set_property("active-pad", &sink).unwrap();
            };
            self.sources.insert(name,sink);
            true
        } else {
            false
        }
    }
}

impl Addressable for Mux {
    fn get_name (&self) -> &str { "mux" }
    fn get_format (&self) -> MsgType { self.format }
}

impl Replybox<Request<String>,Reply<MuxInfo>> for Mux {
    
    fn reply (&self) ->
        Box<Fn(Request<String>)->Result<Reply<MuxInfo>,String> + Send + Sync> {
            let state = self.state.clone();

            Box::new(move |req| {
                let mut state = match state.lock() {
                    Ok(s)   => s,
                    Err(_)  => return Err(String::from("can't acquire mux layout")),
                };

                let mut state = match *state {
                    Some (ref mut s) => s,
                    None         => return Err(String::from("Muxer is not initialized")),
                };
                
                match req {
                    Request::Get => Ok(Reply::Get(state.info())),
                    Request::Set(choice) => match state.apply(choice) {
                        Ok(()) => Ok(Reply::Set),
                        Err(e) => Err(e),
                    }
                }
            })
        }
}

impl Mux {
    pub fn new (format: MsgType, sender: Sender<Vec<u8>>) -> Mux {
        let chat  = Arc::new (Mutex::new (Notifier::new("mux", format, sender )));
        let state = Arc::new (Mutex::new (None));
        Mux { format, chat, state }
    }

    pub fn init (&mut self, pipe: &gst::Pipeline) {
        let muxer = MuxState::new(pipe);
        self.chat.lock().unwrap().talk(&muxer.info());
        *self.state.lock().unwrap() = Some(muxer);
    }

    pub fn reset (&mut self) {
        *self.state.lock().unwrap() = None;
    }

    pub fn plug (&mut self, pad: &SrcPad) {
        match *self.state.lock().unwrap() {
            None => (), // TODO invariant?
            Some(ref mut state) => if state.plug(pad) {
                self.chat.lock().unwrap().talk(&state.info());
            }
        }
    }

    pub fn src_pad (&self) -> gst::Pad {
        match *self.state.lock().unwrap() {
            None => panic!("Audio_mux::src_pad invariant is brocken"), // TODO invariant?
            Some(ref state) => state.selector.get_static_pad("src").unwrap()
        }
    }
}
*/
