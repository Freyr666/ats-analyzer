use serde::{Serialize,Deserialize};
//use signals::Signal;
//use std::sync::{Arc,Mutex};
use std::sync::mpsc::{Sender};
use serde_json;
use serde_msgpack;
use std::str::FromStr;

#[derive(Clone, Copy, Debug)]
pub enum MsgType {
    Msgpack,
    Json
}

#[derive(Serialize)]
struct Msg <'a, T: 'a> {
    name: &'a str,
    data: &'a T,
}

#[derive(Serialize, Deserialize, Debug)]
pub struct Name<'a> {
    pub name: &'a str,
}

#[derive(Deserialize, Debug)]
pub struct Request<'a, T: 'a> {
    pub name: &'a str,
    #[serde(bound(deserialize = "&'a T: Deserialize<'de>"))]
    pub data: &'a T,
}

#[derive(Serialize, Debug)]
pub enum Response<T> {
    Fine (T),
    Error(String)
}

pub trait Addressable {
    fn set_name (&mut self, String);
    fn get_name (&self) -> &str;

    fn set_format (&mut self, MsgType);
    fn get_format (&self) -> MsgType;
}

pub trait Sendbox {
    fn set_sender (&self, Sender<&[u8]>);
    fn get_sender (&self) -> Option<&Sender<&[u8]>>;
}

pub trait Replybox<'a,T,R> {    
    fn reply (&self, &'a T) -> Result<R,String>;    
}

pub trait Notifier<'a, T>: Addressable
    where T: Serialize {
    
    fn serialize_msg (&'a self, data: &'a T) -> Vec<u8> {
        let msg = Msg { name: self.get_name(),
                        data };
        match self.get_format() {
            MsgType::Msgpack => serde_json::to_vec(&msg).unwrap(),
            MsgType::Json    => serde_msgpack::to_vec(&msg).unwrap(),
        }
    }
}

pub trait Chatterer<'a, T>: Notifier<'a, T> + Sendbox
    where T: Addressable + Serialize {
    
    fn connect_channel (&mut self, m: MsgType, s: Sender<&[u8]>) {
        self.set_format(m);
        self.set_sender(s);
    }
    
    fn talk (&'a self, data: &'a T) {
       let s = self.get_sender().unwrap();
       s.send(&self.serialize_msg(data)).unwrap()
    }
}

pub trait Respondent<'a, T, R>: Addressable + Replybox<'a,T,R>
    where &'a T: 'a + Deserialize<'a>,
          R: Serialize {

    fn respond (&self, req: &'a [u8]) -> Vec<u8> {
        let fmt = self.get_format();
        
        let v : Request<T> = match fmt {
            MsgType::Msgpack => serde_msgpack::from_slice(&req).unwrap(),
            MsgType::Json    => serde_json::from_slice(&req).unwrap(),
        };
        
        if v.name != self.get_name() {
            panic!("Respondent: name mismatch")
        };

        let rep = match self.reply(&v.data) {
            Ok(v)  => Response::Fine(v),
            Err(s) => Response::Error(s),
        };

        match fmt {
            MsgType::Msgpack => serde_msgpack::to_vec(&rep).unwrap(),
            MsgType::Json    => serde_json::to_vec(&rep).unwrap(),
        }
    }
        
}

pub trait DispatchTable<'a> {
    fn add_respondent (&mut self, String, Box<Fn(&'a [u8]) -> Vec<u8> + 'a>);
    fn get_respondent (&'a self, &str) -> Option<&Box<Fn(&'a [u8]) -> Vec<u8> + 'a>>;
}

pub trait Dispatcher<'a>: Addressable + DispatchTable<'a> {

    fn add_to_table<'b,T,R> (&'a mut self, r: &'a Respondent<'a,T,R>)
        where &'a T: 'a + Deserialize<'a>,
              R: Serialize {
        let name = String::from_str(r.get_name()).unwrap();
        //let clos = Box::new(move | buf | { r.respond(buf)});
        self.add_respondent(name, Box::new(move | buf | { r.respond(buf)}) )
    }
    
    fn dispatch (&'a self, r: &'a [u8]) -> Option<Vec<u8>> {
        let n : Name = match self.get_format() {
            MsgType::Msgpack => serde_msgpack::from_slice(&r).unwrap(),
            MsgType::Json    => serde_json::from_slice(&r).unwrap(),
        };

        let resp = self.get_respondent(n.name)?;

        Some(resp(r))
    }
    
}

/*
pub trait Chatterer<'a,T>
    where T: Serialize + Deserialize<'a> {

    fn name(&self) -> &'static str;

    fn ask_state(&'a self) -> &'a T;
    fn set_state(&self, b: &T) -> Response;
    fn signal(&self) -> Arc<Mutex<Signal<&'a T>>>;
}
*/
