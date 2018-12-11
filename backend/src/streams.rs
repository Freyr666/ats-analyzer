use metadata::Structure;
use std::sync::{Arc,Mutex};
use std::sync::mpsc::Sender;
use chatterer::notif::Notifier;
use signals::Msg;
use probe::Probe;


pub struct StreamParser {
    pub structures: Arc<Mutex<Vec<Structure>>>,
    chat: Arc<Mutex<Notifier>>,
}

impl StreamParser {
    pub fn new (sender: Sender<Vec<u8>>) -> StreamParser {
        let chat       = Arc::new(Mutex::new( Notifier::new("stream_parser", sender )));
        let structures = Arc::new(Mutex::new(vec![]));
        StreamParser { chat, structures }
    }

    // TODO remove update
    fn set_data (chat: &Notifier, structures: &mut Vec<Structure>, s: &Structure) {
        if structures.is_empty()
            || ! (structures.iter().any(|st| st.id == s.id)) {
                structures.push(s.clone())
            } else {
                let str = structures.iter_mut().find(|st| st.id == s.id).unwrap();
                str.from(s)
            }
        chat.talk(&structures); // TODO chack var
    }
    
    pub fn connect_probe (&mut self, p: &mut Probe) {
        let chat    = self.chat.clone();
        let structs = self.structures.clone();
        p.updated.lock().unwrap().connect(move |s| {
            StreamParser::set_data(&chat.lock().unwrap(),
                                   &mut structs.lock().unwrap(),
                                   s);
        } );
    }
}
