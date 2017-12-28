use chatterer::{Chatterer,Response};
use serde::{Serialize,Deserialize};
use std::sync::{Arc,Mutex};
use signals::Signal;

#[derive(Serialize,Deserialize,Debug)]
pub struct GraphSettings {
    x: i32
}

pub struct Graph<'a> {
    signal:       Arc<Mutex<Signal<&'a GraphSettings>>>,
    pub settings: &'a GraphSettings
}

impl<'a> Graph<'a> {
    pub fn new() -> Result<Graph<'a>,String> {
        Ok(Graph { signal:   Arc::new(Mutex::new(Signal::new())),
                   settings: &GraphSettings { x: 42 } })
    }

    //pub fn run();
}

// impl<'a,GraphSettings> Chatterer<'a,GraphSettings> for Graph<'a>
//     where GraphSettings: Serialize+Deserialize<'a>{

//     fn name(&self) -> &'static str {
//         "graph"
//     }

//     fn ask_state(&'a self) -> &'a GraphSettings {
//         let a : &'a GraphSettings = self.settings;
//         a
//     }

//     fn set_state(&self, new_state: &GraphSettings) -> Response {
//         Response::Fine
//     }

//     fn signal(&self) -> Arc<Mutex<Signal<&'a GraphSettings>>> {
//         self.signal
//     }
// }
