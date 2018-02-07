use metadata::Structure;
use std::sync::{Arc,Mutex};
use chatterer::{Notifier,Addressable,Sendbox,Replybox,MsgType};
use signals::Msg;
use std::sync::mpsc::Sender;
use std::str::FromStr;
use probe::Probe;

pub struct StreamsState {
    format:     MsgType,
    sender:     Option<Sender<Vec<u8>>>,
    structures: Arc<Mutex<Vec<Structure>>>,
    update:     Arc<Mutex<Msg<Vec<Structure>,Result<(),String>>>>,
}

pub struct Streams {
    pub state:  Arc<Mutex<StreamsState>>,
}

impl Addressable for StreamsState {
    fn get_name (&self) -> &str { "streams" }

    fn set_format (&mut self, t: MsgType) { self.format = t }
    fn get_format (&self) -> MsgType { self.format }
}

impl<'a> Sendbox<'a> for StreamsState {
    fn set_sender (&mut self, s: Sender<Vec<u8>>) { self.sender = Some(s) }
    fn get_sender (&'a self) -> Option<&'a Sender<Vec<u8>>> {
        match self.sender {
            None => None,
            Some(ref s) => Some(&s)
        }
    }
}

impl Replybox<Vec<Structure>, ()> for StreamsState {
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

impl<'a> Notifier<'a, Vec<Structure>> for StreamsState { }

impl Streams {
    pub fn new () -> Streams {
        let update = Arc::new(Mutex::new(Msg::new()));
        let format = MsgType::Json;
        let structures = Arc::new(Mutex::new(vec![]));
        let state  = Arc::new(Mutex::new(StreamsState { update, format, sender: None, structures }));
        Streams { state }
    }

    pub fn connect_channel(&mut self, m: MsgType, s: Sender<Vec<u8>>) {
        let mut chat = self.state.lock().unwrap();
        chat.set_format(m);
        chat.set_sender(s);
    }

    pub fn update (&self) -> Arc<Mutex<Msg<Vec<Structure>,Result<(),String>>>> {
        self.state.lock().unwrap().update.clone()
    }

    fn set_data (state: &StreamsState, s: &Structure) {
        let mut structures = state.structures.lock().unwrap();
        if structures.is_empty()
            || ! (structures.iter().any(|st| st.id == s.id)) {
                structures.push(s.clone())
            } else {
                let str = structures.iter_mut().find(|st| st.id == s.id).unwrap();
                str.from(s)
            }
        state.talk(&structures);
    }
    
    pub fn connect_probe (&mut self, p: &mut Probe) {
        let state = self.state.clone();
        p.updated.lock().unwrap().connect(move |s| {
            Streams::set_data(&state.lock().unwrap(), s);
        } );
    }
}
