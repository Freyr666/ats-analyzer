use metadata::Structure;
use std::sync::{Arc,Mutex};
use chatterer::{Chatterer,Notifier,Addressable,Sendbox,Replybox,MsgType};
use signals::Msg;
use std::sync::mpsc::Sender;
use std::str::FromStr;
use probe::Probe;
use std::marker::PhantomData;

pub struct StreamsComm {
    name:       String,
    format:     MsgType,
    sender:     Option<Sender<Vec<u8>>>,
    update:     Arc<Mutex<Msg<Vec<Structure>,Result<(),String>>>>,
    
    //phantom: PhantomData<&'a i32>
}

pub struct Streams {
    pub chatterer:  Arc<Mutex<StreamsComm>>,
    
    structures: Arc<Mutex<Vec<Structure>>>,
}

impl<'a> Addressable for StreamsComm {
    fn get_name (&self) -> &str { &self.name }

    fn set_format (&mut self, t: MsgType) { self.format = t }
    fn get_format (&self) -> MsgType { self.format }
}

impl<'a> Sendbox<'a> for StreamsComm {
    fn set_sender (&mut self, s: Sender<Vec<u8>>) { self.sender = Some(s) }
    fn get_sender (&'a self) -> Option<&'a Sender<Vec<u8>>> {
        match self.sender {
            None => None,
            Some(ref s) => Some(&s)
        }
    }
}

impl Replybox<Vec<Structure>, ()> for StreamsComm {
    fn reply (&self) -> Box<Fn(Vec<Structure>)->Result<(),String> + Send + Sync> {
        let signal = self.update.clone();
        Box::new(move | data: Vec<Structure>| {
            match signal.lock().unwrap().emit(data) {
                None    => Err(String::from_str("Streams are not connected to the graph").unwrap()),
                Some(r) => r
            }
        })
    }
}

impl<'a> Notifier<'a, Vec<Structure>> for StreamsComm { }

impl<'a> Chatterer<'a, Vec<Structure>> for StreamsComm { }

impl Streams {
    pub fn new () -> Streams {
        let name      = String::from_str("streams").unwrap();
        let update    = Arc::new(Mutex::new(Msg::new()));
        let chatterer = Arc::new(Mutex::new(StreamsComm { name, update, format: MsgType::Json, sender: None }));
        Streams { chatterer, structures: Arc::new(Mutex::new(vec![])) }
    }

    pub fn connect_channel(&mut self, m: MsgType, s: Sender<Vec<u8>>) {
        let mut chat = self.chatterer.lock().unwrap();
        chat.set_format(m);
        chat.set_sender(s);
    }

    pub fn update (&self) -> Arc<Mutex<Msg<Vec<Structure>,Result<(),String>>>> {
        self.chatterer.lock().unwrap().update.clone()
    }

    fn set_data (structures: &mut Vec<Structure>, chatterer: &StreamsComm, s: &Structure) {
        if structures.is_empty()
            || ! (structures.iter().any(|st| st.id == s.id)) {
                structures.push(s.clone())
            } else {
                let str = structures.iter_mut().find(|st| st.id == s.id).unwrap();
                str.from(s)
            }
        chatterer.talk(structures);
        chatterer.update.lock().unwrap().emit(structures.clone()); // TODO remove this
    }
    
    pub fn connect_probe (&mut self, p: &mut Probe) {
        let structs = self.structures.clone();
        let chatterer = self.chatterer.clone();
        p.updated.lock().unwrap().connect(move |s| {
            Streams::set_data(&mut structs.lock().unwrap(), &chatterer.lock().unwrap(), s);
        } );
    }
}
