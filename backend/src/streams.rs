use metadata::Structure;
use std::sync::{Arc,Mutex};
use chatterer::{Notifier,Addressable,Sendbox,Replybox,MsgType};
use signals::Msg;
use std::sync::mpsc::Sender;
use std::str::FromStr;
use probe::Probe;

pub struct StreamsChatterer {
    format:     MsgType,
    sender:     Sender<Vec<u8>>,
}

pub struct Streams {
    format:     MsgType,
    structures: Arc<Mutex<Vec<Structure>>>,

    pub chat:   Arc<Mutex<StreamsChatterer>>,
    pub update: Arc<Mutex<Msg<Vec<Structure>,Result<(),String>>>>,
}

impl Addressable for StreamsChatterer {
    fn get_name (&self) -> &str { "streams" }
    fn get_format (&self) -> MsgType { self.format }
}

impl Addressable for Streams {
    fn get_name (&self) -> &str { "streams" }
    fn get_format (&self) -> MsgType { self.format }
}

impl Sendbox for StreamsChatterer {
    fn get_sender (&self) -> &Sender<Vec<u8>> { &self.sender }
}

impl Replybox<Vec<Structure>, ()> for Streams {
    fn reply (&self) -> Box<Fn(Vec<Structure>)->Result<(),String> + Send + Sync> {
        let signal = self.update.clone();
        let structures = self.structures.clone();
        Box::new(move | data: Vec<Structure>| {
            let mut s = structures.lock().unwrap();
            *s = data.clone();
            match signal.lock().unwrap().emit(data) {
                None    => Err(String::from_str("Streams are not connected to the graph").unwrap()),
                Some(r) => r
            }
        })
    }
}

impl Notifier<Vec<Structure>> for StreamsChatterer { }

impl Streams {
    pub fn new (format: MsgType, sender: Sender<Vec<u8>>) -> Streams {
        let update     = Arc::new(Mutex::new(Msg::new()));
        let structures = Arc::new(Mutex::new(vec![]));
        let chat       = Arc::new(Mutex::new(StreamsChatterer { format, sender }));
        Streams { format, chat, update, structures }
    }

    fn set_data (chat: &StreamsChatterer, structures: &mut Vec<Structure>, s: &Structure) {
        if structures.is_empty()
            || ! (structures.iter().any(|st| st.id == s.id)) {
                structures.push(s.clone())
            } else {
                let str = structures.iter_mut().find(|st| st.id == s.id).unwrap();
                str.from(s)
            }
        chat.talk(&structures);
    }
    
    pub fn connect_probe (&mut self, p: &mut Probe) {
        let chat    = self.chat.clone();
        let structs = self.structures.clone();
        p.updated.lock().unwrap().connect(move |s| {
            Streams::set_data(&chat.lock().unwrap(), &mut structs.lock().unwrap(), s);
        } );
    }
}
