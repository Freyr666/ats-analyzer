use std::collections::HashMap;
use std::vec::Vec;
use std::sync::{Arc,Mutex};
use std::sync::mpsc::Sender;
use chatterer::MsgType;
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
    format:      MsgType,
    pub chat:    Arc<Mutex<Notifier>>,
    state:       Arc<Mutex<MuxState>>,
}

impl MuxState {
    fn new (pipe: gst::Pipeline) -> MuxState {
        let sources  = HashMap::new ();
        let chosen   = None;
        let selector = gst::ElementFactory::make("input-selector", None).unwrap();
        pipe.add_many(&[&selector]).unwrap();
        MuxState { sources, chosen, selector }
    }

    fn info (&self) -> MuxInfo {
        let chosen  = self.chosen.clone ();
        let mut sources = Vec::with_capacity (self.sources.capacity());
        for (s,_) in &self.sources {
            sources.push(s.clone());
        };
        MuxInfo { chosen, sources }
    }

    fn apply (&mut self, choice: String) -> Result<(),String> {
        if self.chosen.as_ref().map_or(true, |x| { *x != choice }) {
            if let Some(p) = self.sources.get(&choice) {
                self.chosen = Some(choice);
                self.selector.set_property("active-pad", &p).unwrap();
                Ok(())
            } else {
                Err(String::from("Pid not found"))
            }
        } else {
            Ok (())
        }
    }

    fn plug (&mut self, pad: &SrcPad) -> bool {
        let name = format!("{}_{}_({})", pad.pid, pad.channel, pad.stream);
        if ! self.sources.contains_key (&name) {
            let sink = self.selector.get_request_pad("sink_%u").unwrap();
            pad.pad.link(&sink);
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
                match req {
                    Request::Get =>
                        if let Ok(s) = state.lock() {
                            Ok(Reply::Get(s.info()))
                        } else {
                            Err(String::from("can't acquire mux layout"))
                        },
                    Request::Set(choice) =>
                        if let Ok(mut s) = state.lock() {
                            match s.apply(choice) {
                                Ok(()) => Ok(Reply::Set),
                                Err(e) => Err(e),
                            }
                        } else {
                            Err(String::from("can't acquire mux layout"))
                        },
                }
            })
        }
}

impl Mux {
    pub fn new (pipe: gst::Pipeline, format: MsgType, sender: Sender<Vec<u8>>) -> Mux {
        let chat  = Arc::new (Mutex::new (Notifier::new("mux", format, sender )));
        let state = Arc::new (Mutex::new (MuxState::new(pipe)));
        Mux { format, chat, state }
    }

    pub fn reset (&mut self, pipe: gst::Pipeline) {
        *self.state.lock().unwrap() = MuxState::new(pipe);
        self.chat.lock().unwrap().talk(&self.state.lock().unwrap().info());
    }

    pub fn plug (&mut self, pad: &SrcPad) {
        if self.state.lock().unwrap().plug(pad) {
            self.chat.lock().unwrap().talk(&self.state.lock().unwrap().info());
        }
    }

    pub fn src_pad (&self) -> gst::Pad {
        self.state.lock().unwrap().selector.get_static_pad("src").unwrap()
    }
}
