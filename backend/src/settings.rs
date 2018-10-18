use std::sync::{Arc,Mutex};
use chatterer::MsgType;
use chatterer::notif::Notifier;
use chatterer::control::{Addressable,Replybox};
use chatterer::control::message::{Request,Reply};
use signals::Msg;
use std::sync::mpsc::Sender;

#[derive(Copy,Clone,PartialEq,Serialize,Deserialize,Debug)]
pub struct Setting {
    pub peak_en : bool,
    pub peak    : f32,
    pub cont_en : bool,
    pub cont    : f32,
    pub duration: f32,
}

#[derive(Copy,Clone,PartialEq,Serialize,Deserialize,Debug)]
pub struct Black {
    pub black       : Setting,
    pub luma        : Setting,
    pub black_pixel : u32,
}

#[derive(Copy,Clone,PartialEq,Serialize,Deserialize,Debug)]
pub struct Freeze {
    pub freeze     : Setting,
    pub diff       : Setting,
    pub pixel_diff : u32,
}

#[derive(Copy,Clone,PartialEq,Serialize,Deserialize,Debug)]
pub struct Blocky {
    pub blocky      : Setting,
}

#[derive(Copy,Clone,PartialEq,Serialize,Deserialize,Debug)]
pub struct Silence {
    pub silence : Setting,
}

#[derive(Copy,Clone,PartialEq,Serialize,Deserialize,Debug)]
pub struct Loudness {
    pub loudness : Setting,
}

#[derive(Copy,Clone,PartialEq,Serialize,Deserialize,Debug)]
pub struct Adv {
    pub adv_diff : f32,
    pub adv_buf  : i32,
}

#[derive(Copy,Clone,PartialEq,Serialize,Deserialize,Debug)]
pub struct Video {
    pub loss   : f32,
    pub black  : Black,
    pub freeze : Freeze,
    pub blocky : Blocky,
}

#[derive(Copy,Clone,PartialEq,Serialize,Deserialize,Debug)]
pub struct Audio {
    pub loss     : f32,
    pub silence  : Silence,
    pub loudness : Loudness,
    pub adv      : Adv
}

#[derive(Copy,Clone,PartialEq,Serialize,Deserialize,Debug)]
pub struct Settings {
    pub video : Video,
    pub audio : Audio,
}

pub struct Configuration {
    format:     MsgType,
    settings:   Arc<Mutex<Option<Settings>>>,

    pub chat:   Arc<Mutex<Notifier>>,
    pub update: Arc<Mutex<Msg<Settings,Result<(),String>>>>,
}

impl Addressable for Configuration {
    fn get_name (&self) -> &str { "settings" }
    fn get_format (&self) -> MsgType { self.format }
}

impl Replybox<Request<Settings>,Reply<Settings>> for Configuration {

    fn reply (&self) ->
        Box<Fn(Request<Settings>)->Result<Reply<Settings>,String> + Send + Sync> {
            let signal   = self.update.clone();
            let settings = self.settings.clone();
            Box::new(move | data: Request<Settings> | {
                match data {
                    Request::Get =>
                        if let Ok(s) = settings.lock() {
                            match *s {
                                Some(ref s) => Ok(Reply::Get(*s)),
                                None        => Err(String::from("no settings available"))
                            }
                        } else {
                            Err(String::from("can't acquire the settings"))
                        },
                    Request::Set(data) => {
                        let mut s = settings.lock().unwrap();
                        *s = Some(data);
                        match signal.lock().unwrap().emit(data) {
                            None    => Err(String::from("Settings are not connected to the graph")),
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

impl Configuration {
    pub fn new (format: MsgType, sender: Sender<Vec<u8>>) -> Configuration {
        let update     = Arc::new(Mutex::new(Msg::new()));
        let settings   = Arc::new(Mutex::new(None));
        let chat       = Arc::new(Mutex::new(Notifier::new("settings", format, sender )));

        Configuration { format, chat, update, settings }
    }
}
