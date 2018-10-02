use metadata::Structure;
use std::sync::{Arc,Mutex};
use chatterer::MsgType;
use chatterer::notif::Notifier;
use chatterer::control::{Addressable,Replybox};
use chatterer::control::message::{Request,Reply};
use signals::Msg;
use std::sync::mpsc::Sender;
use probe::Probe;


pub struct Streams {
    format:     MsgType,
    structures: Arc<Mutex<Vec<Structure>>>,

    pub chat:   Arc<Mutex<Notifier>>,
    pub update: Arc<Mutex<Msg<Vec<Structure>,Result<(),String>>>>,
}

impl Addressable for Streams {
    fn get_name (&self) -> &str { "structures" }
    fn get_format (&self) -> MsgType { self.format }
}

impl Replybox<Request<Vec<Structure>>,Reply<Vec<Structure>>> for Streams {
    
    fn reply (&self) ->
        Box<Fn(Request<Vec<Structure>>)->Result<Reply<Vec<Structure>>,String> + Send + Sync> {
            
            let signal = self.update.clone();
            let structures = self.structures.clone();
            Box::new(move | data: Request<Vec<Structure>>| {
                match data {
                    Request::Get => 
                        if let Ok(s) = structures.lock() {
                            Ok(Reply::Get(s.clone()))
                        } else {
                            Err(String::from("can't acquire the structure"))
                        },
                    Request::Set(data) => {
                        let mut s = structures.lock().unwrap();
                        *s = data.clone();
                        debug!("Streams::set");
                        match signal.lock().unwrap().emit(data) {
                            None    => Err(String::from("Streams are not connected to the graph")),
                            Some(r) => match r {
                                Ok(()) => Ok(Reply::Set),
                                Err(e) => Err(e),
                            }
                        }
                    }
                }
            })
        }
}

impl Streams {
    pub fn new (format: MsgType, sender: Sender<Vec<u8>>) -> Streams {
        let update     = Arc::new(Mutex::new(Msg::new()));
        let structures = Arc::new(Mutex::new(vec![]));
        let chat       = Arc::new(Mutex::new(Notifier::new("structures", format, sender )));
        Streams { format, chat, update, structures }
    }

    // TODO remove update
    fn set_data (chat: &Notifier, structures: &mut Vec<Structure>, s: &Structure, _update: &Msg<Vec<Structure>,Result<(),String>>) {
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
        let msg     = self.update.clone();
        p.updated.lock().unwrap().connect(move |s| {
            Streams::set_data(&chat.lock().unwrap(), &mut structs.lock().unwrap(), s, &msg.lock().unwrap());
        } );
    }
}
