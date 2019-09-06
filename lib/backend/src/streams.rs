use serde_json;
use std::sync::{Arc,Mutex};
use std::sync::mpsc::Sender;
use media_stream::structure::Structure;
use signals::Signal;
use probe::Probe;

pub struct StreamParser {
    pub structures: Arc<Mutex<Vec<Structure>>>,
    sender: Arc<Mutex<Sender<Vec<u8>>>>,
}

impl StreamParser {
    pub fn new (sender: Sender<Vec<u8>>) -> StreamParser {
        let structures = Arc::new(Mutex::new(vec![]));
        let sender     = Arc::new(Mutex::new(sender));
        StreamParser { sender, structures }
    }

    // TODO remove update
    fn set_data (signal: &Sender<Vec<u8>>, structures: &mut Vec<Structure>, s: &Structure) {
        if structures.is_empty()
            || ! (structures.iter().any(|st| st.id == s.id)) {
                structures.push(s.clone())
            } else {
                let str = structures.iter_mut().find(|st| st.id == s.id).unwrap();
                str.from(s)
            }
        signal.send (serde_json::to_vec(&structures).unwrap());
       // chat.talk(&structures); // TODO chack var
    }
    
    pub fn connect_probe (&mut self, p: &mut Probe) {
        let sender  = Arc::downgrade(&self.sender);
        let structs = self.structures.clone ();
        p.updated.lock().unwrap().connect(move |s| {
            match sender.upgrade() {
                None => (),
                Some (sender) => 
                    StreamParser::set_data(&sender.lock().unwrap(),
                                           &mut structs.lock().unwrap(),
                                           s),
            };
        });
    }
/*
    pub fn connect_streams_changed<F> (&mut self, f: F)
    where F: Fn(Vec<u8>) + Send + Sync + 'static {
        self.signal.lock().unwrap().connect(f);
    }
*/
}
