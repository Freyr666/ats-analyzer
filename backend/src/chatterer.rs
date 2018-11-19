#[derive(Clone, Copy, Debug)]
pub enum MsgType {
    Msgpack,
    Json
}

pub trait Description {
    fn describe () -> String;
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
            // debug!("Notifier::{} has sent a notification", self.name);
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
    pub struct Name<'a> {
        pub name: &'a str,
    }

    #[derive(Deserialize, Debug)]
    pub struct Method<'a> {
        pub name:    &'a str,
        pub method:  &'a str,
        pub counter: i32,
    }

    #[derive(Deserialize, Debug)]
    pub struct Content<T> {
        pub content: T,
    }
    
    #[derive(Serialize, Debug)]
    pub struct Response<'a,T> {
        pub name:    &'a str,
        pub method:  &'a str,
        pub counter: i32,
        pub result:  Result<T,String>,
    }

    pub fn parse<'a,T> (format: &MsgType, msg: &'a Vec<u8>) -> T
    where T: serde::Deserialize<'a> {
        match format {
            MsgType::Json    => serde_json::from_slice(&msg).unwrap(),
            MsgType::Msgpack => serde_msgpack::from_slice(&msg).unwrap(),
        }
    }

    impl<'a> Method<'a> {
        pub fn respond<T> (&self, result: Result<T,String>)
                        -> Response<'a,T> {
            Response { name: self.name, method: self.method, counter: self.counter, result }
        }

        pub fn respond_ok<T> (&self, v: T) -> Response<'a,T> {
            Response { name: self.name, method: self.method,
                       counter: self.counter, result: Ok(v) }
        }

        pub fn respond_err<T> (&self, v: &str) -> Response<'a,T> {
            Response { name: self.name, method: self.method,
                       counter: self.counter, result: Err(String::from(v)) }
        }
            
    }

    impl<'a,T> Response<'a,T>
    where T: serde::Serialize {
        pub fn serialize (&self, format: &MsgType) -> Vec<u8> {
            match format {
                MsgType::Json    => serde_json::to_vec(&self).unwrap(),
                MsgType::Msgpack => serde_msgpack::to_vec(&self).unwrap(),
            }
        }
    }
}
