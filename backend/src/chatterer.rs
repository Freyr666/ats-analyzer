use serde::{Serialize,Deserialize};
use serde::de::DeserializeOwned;
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

pub trait Addressable {
    fn get_name (&self) -> &str;

    fn set_format (&mut self, MsgType);
    fn get_format (&self) -> MsgType;
}

pub trait Sendbox<'a> {
    fn set_sender (&mut self, Sender<Vec<u8>>);
    fn get_sender (&'a self) -> Option<&'a Sender<Vec<u8>>>;
}

pub trait Replybox<T,R>: Addressable
    where T: DeserializeOwned,
          R: Serialize {    
    fn reply (&self) -> Box<Fn(T)->Result<R,String> + Send + Sync>;
}

pub trait Notifier<'a, T>: Addressable + Sendbox<'a>
    where T: Serialize {

    fn serialize_msg (&'a self, data: &'a T) -> Vec<u8> {
        let msg = Msg { name: self.get_name(),
                        data };
        match self.get_format() {
            MsgType::Json    => serde_json::to_vec(&msg).unwrap(),
            MsgType::Msgpack => serde_msgpack::to_vec(&msg).unwrap(),
        }
    }
    
    fn connect_channel (&mut self, m: MsgType, s: Sender<Vec<u8>>) {
        self.set_format(m);
        self.set_sender(s);
    }
    
    fn talk (&'a self, data: &'a T) {
        let s = self.get_sender().unwrap();
        s.send(self.serialize_msg(data)).unwrap()
    }
}

pub trait DispatchTable {
    fn add_respondent (&mut self, String, Box<Fn(Vec<u8>) -> Vec<u8> + Sync + Send>);
    fn get_respondent (&self, &str) -> Option<&Box<Fn(Vec<u8>) -> Vec<u8> + Sync + Send>>;
}

pub trait Dispatcher: Addressable + DispatchTable {

    fn add_to_table<T,R> (&mut self, r: &Replybox<T,R>)
        where T: 'static + DeserializeOwned,
              R: 'static + Serialize {
        let name  = String::from_str(r.get_name()).unwrap();
        let fmt   = self.get_format();
        let rep   = r.reply();
        
        let clos = move | buf: Vec<u8> | { 
            let req : Request<T> = match fmt {
                MsgType::Json    => serde_json::from_slice(&buf).unwrap(),
                MsgType::Msgpack => serde_msgpack::from_slice(&buf).unwrap(),
            };

            let rep : Response<R> = match rep(req.data) {
                Ok(v)  => Response::Fine(v),
                Err(s) => Response::Error(s),
            };

            match fmt {
                MsgType::Msgpack => serde_msgpack::to_vec(&rep).unwrap(),
                MsgType::Json    => serde_json::to_vec(&rep).unwrap(),
            }
        };
        self.add_respondent(name, Box::new(move | buf | { clos(buf)}));
    }
    
    fn dispatch (&self, r: Vec<u8>) -> Option<Vec<u8>> {
        let n : Name = match self.get_format() {
            MsgType::Json    => serde_json::from_slice(&r).unwrap(),
            MsgType::Msgpack => serde_msgpack::from_slice(&r).unwrap(),
        };
        println!("name: {}", n.name);
        let resp = self.get_respondent(&n.name)?;
        
        Some(resp(r))
    }
    
}
