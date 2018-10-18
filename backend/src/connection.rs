use chatterer::MsgType;
use chatterer::control::{Addressable,Replybox};
use signals::Signal;
use std::sync::{Arc,Mutex};

pub struct Conn {
    format: MsgType,
    signal: Arc<Mutex<Signal<()>>>,
}

impl Conn {
    pub fn new (format: MsgType) -> Conn {
        let signal = Arc::new(Mutex::new(Signal::new()));
        Conn { format, signal }
    }

    pub fn connect<F> (&self, cb: F)
    where F: Fn(&()) + Send + Sync + 'static {
        self.signal.lock().unwrap().connect(cb);
    }
}

impl  Addressable for Conn {
    fn get_name(&self) -> &str { "connection" }
    fn get_format(&self) -> MsgType { self.format }
}

impl Replybox<(),String> for Conn {
    fn reply (&self) -> Box<Fn(())->Result<String,String> + Send + Sync> {
        let signal = self.signal.clone();
        Box::new(move |_| {
            signal.lock().unwrap().emit(&());
            Ok(String::from("established"))
        })
    }
}
