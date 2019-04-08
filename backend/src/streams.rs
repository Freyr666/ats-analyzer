use serde_json;
use std::sync::{Arc,Mutex};
use std::sync::mpsc::Sender;
use metadata::Structure;
use signals::Signal;
use probe::Probe;

pub struct StreamParser {
    pub structures: Arc<Mutex<Vec<Structure>>>,
    signal: Arc<Mutex<Signal<Vec<u8>>>>,
}

pub trait Api {
    fn connect_streams_changed<F> (&mut self, f: F)
    where F: Fn(&Vec<u8>) + Send + Sync + 'static;
}

impl StreamParser {
    pub fn new () -> StreamParser {
        let signal     = Arc::new(Mutex::new( Signal::new() ));
        let structures = Arc::new(Mutex::new(vec![]));
        StreamParser { signal, structures }
    }

    // TODO remove update
    fn set_data (signal: &Signal<Vec<u8>>, structures: &mut Vec<Structure>, s: &Structure) {
        if structures.is_empty()
            || ! (structures.iter().any(|st| st.id == s.id)) {
                structures.push(s.clone())
            } else {
                let str = structures.iter_mut().find(|st| st.id == s.id).unwrap();
                str.from(s)
            }
        signal.emit (&serde_json::to_vec(&structures).unwrap());
       // chat.talk(&structures); // TODO chack var
    }
    
    pub fn connect_probe (&mut self, p: &mut Probe) {
        let signal  = self.signal.clone();
        let structs = self.structures.clone();
        p.updated.lock().unwrap().connect(move |s| {
            StreamParser::set_data(&signal.lock().unwrap(),
                                   &mut structs.lock().unwrap(),
                                   s);
        } );
    }
}

impl Api for StreamParser {
    fn connect_streams_changed<F> (&mut self, f: F)
    where F: Fn(&Vec<u8>) + Send + Sync + 'static {
        self.signal.lock().unwrap().connect(f);
    }
}
