use serde::{Serialize,Deserialize};
use signals::Signal;
use std::sync::{Arc,Mutex};

#[derive(Serialize, Deserialize, Debug)]
pub enum Response {
    Fine,
    Error(String)
}

pub trait Chatterer<'a,T>
    where T: Serialize + Deserialize<'a> {

    fn name(&self) -> &'static str;

    fn ask_state(&'a self) -> &'a T;
    fn set_state(&self, b: &T) -> Response;
    fn signal(&self) -> Arc<Mutex<Signal<&'a T>>>;
}
