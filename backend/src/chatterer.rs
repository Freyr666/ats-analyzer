#[derive(Clone, Copy, Debug)]
pub enum MsgType {
    Msgpack,
    Json
}

pub mod notif {
    use serde_json;
    use serde_msgpack;
    use serde::Serialize;
    use std::sync::mpsc::Sender;
    use chatterer::MsgType;

    #[derive(Serialize)]
    struct Msg <'a, T: 'a> {
        name: &'a str,
        data: &'a T,
    }

    pub struct Notifier {
        name:   &'static str,
        format: MsgType,
        sender: Sender<Vec<u8>>
    }

    impl Notifier {

        pub fn new(name: &'static str, format: MsgType, sender: Sender<Vec<u8>>) -> Notifier {
            Notifier { name, format, sender }
        }
        
        fn serialize_msg<T> (&self, data: &T) -> Vec<u8>
            where T: Serialize {
            let msg = Msg { name: self.name,
                            data };
            match self.format {
                MsgType::Json    => serde_json::to_vec(&msg).unwrap(),
                MsgType::Msgpack => serde_msgpack::to_vec(&msg).unwrap(),
            }
        }
        
        pub fn talk<T> (&self, data: &T)
        where T: Serialize {
            debug!("Notifier::{} has sent a notification", self.name);
            self.sender.send(self.serialize_msg(data)).unwrap()
        }    
    }
}

pub mod control {
    use serde_json;
    use serde_msgpack;
    use serde::Serialize;
    use serde::de::DeserializeOwned;
    use chatterer::MsgType;
    
    #[derive(Deserialize, Debug)]
    pub struct Name {
        pub name: String,
    }

    #[derive(Deserialize, Debug)]
    pub struct Request<T> {
        pub name: String,
        //#[serde(bound(deserialize = "&'a T: Deserialize<'de>"))]
        pub data: T,
    }

    #[derive(Serialize, Debug)]
    pub enum Response<T> {
        Fine (T),
        Error(String)
    }

    pub mod message {        
        #[derive(Deserialize, Debug)]
        #[serde(tag = "method", content = "body")]
        pub enum Request<T> {
            Get,
            Set(T),
        }

        #[derive(Serialize, Debug)]
        #[serde(tag = "method", content = "body")]
        pub enum Reply<T> {
            Get(T),
            Set,
        }   
    }

    pub trait Addressable {
        fn get_name (&self) -> &str;
        fn get_format (&self) -> MsgType;
    }
    
    pub trait Replybox<T,R>: Addressable
        where T: DeserializeOwned,
              R: Serialize {    
        fn reply (&self) -> Box<Fn(T)->Result<R,String> + Send + Sync>;
    }
    
    pub trait DispatchTable {
        fn add_respondent (&mut self, String, Box<Fn(Vec<u8>) -> Vec<u8> + Sync + Send>);
        fn get_respondent (&self, &str) -> Option<&Box<Fn(Vec<u8>) -> Vec<u8> + Sync + Send>>;
    }

    pub trait Dispatcher: Addressable + DispatchTable {

        fn add_to_table<T,R> (&mut self, r: &Replybox<T,R>)
            where T: 'static + DeserializeOwned,
                  R: 'static + Serialize {
            let name  = String::from(r.get_name());
            let debug_name = name.clone();
            let fmt   = self.get_format();
            let rep   = r.reply();
            
            let clos = move | buf: Vec<u8> | {
                debug!("Replybox::{} was asked", debug_name);
                
                let req : Request<T> = match fmt {
                    MsgType::Json    => serde_json::from_slice(&buf).unwrap(),
                    MsgType::Msgpack => serde_msgpack::from_slice(&buf).unwrap(),
                };

                let rep : Response<R> = match rep(req.data) {
                    Ok(v)  => { debug!("Replybox::{} has answered", debug_name); Response::Fine(v)},
                    Err(s) => { error!("Replybox::{} error {}", debug_name, s); Response::Error(s)},
                };

                match fmt {
                    MsgType::Msgpack => serde_msgpack::to_vec(&rep).unwrap(),
                    MsgType::Json    => serde_json::to_vec(&rep).unwrap(),
                }
            };
            self.add_respondent(name, Box::new(move | buf | { clos(buf)}));
        }
        
        fn dispatch (&self, r: Vec<u8>) -> Option<Vec<u8>> {
            debug!("Dispatcher::dispatch got message");
            let n : Name = match self.get_format() {
                MsgType::Json    => serde_json::from_slice(&r).unwrap(),
                MsgType::Msgpack => serde_msgpack::from_slice(&r).unwrap(),
            };
            debug!("Dispatcher::dispatch got message for {}", n.name);
            let resp = self.get_respondent(&n.name)?;
            
            Some(resp(r))
        }
    }
}
