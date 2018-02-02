use metadata::Structure;
use std::sync::{Arc,Mutex};
use chatterer::{Chatterer,Notifier,Addressable,Sendbox,MsgType};
use std::sync::mpsc::Sender;
use probe::Probe;

pub struct Streams {
    name:       String,
    format:     MsgType,
    sender:     Option<Sender<Vec<u8>>>,
    
    structures: Arc<Mutex<Vec<Structure>>>
}

impl Addressable for Streams {
    fn get_name (&self) -> &str { &self.name }

    fn set_format (&mut self, t: MsgType) { self.format = t }
    fn get_format (&self) -> MsgType { self.format }
}

impl<'a> Sendbox<'a> for Streams {
    fn set_sender (&mut self, s: Sender<Vec<u8>>) { self.sender = Some(s) }
    fn get_sender (&'a self) -> Option<&'a Sender<Vec<u8>>> {
        match self.sender {
            None => None,
            Some(ref s) => Some(&s)
        }
    }
}

impl<'a> Notifier<'a, Vec<Structure>> for Streams { }

impl<'a> Chatterer<'a, Vec<Structure>> for Streams { }

impl Streams {
    pub fn new (name: String) -> Streams {
        Streams { name, format: MsgType::Json, sender: None, structures: Arc::new(Mutex::new(vec![])) }
    }

    fn set_data (structures: &mut Vec<Structure>, s: &Structure) {
        if structures.is_empty()
            || ! (structures.iter().any(|st| st.id == s.id)) {
                structures.push(s.clone())
            } else {
                let str = structures.iter_mut().find(|st| st.id == s.id).unwrap();
                str.from(s)
            }
    }
    
    pub fn connect_probe (&mut self, p: &mut Probe) {
        let structs = self.structures.clone();
        p.updated.lock().unwrap().connect(move |s| {
            Streams::set_data(&mut structs.lock().unwrap() , s) }
        );
    }
}
