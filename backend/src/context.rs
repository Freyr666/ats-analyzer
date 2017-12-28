//use chatterer::ChattererProxy;
//use chatterer::Logger;
use chatterer::Chatterer;
use initial::Initial;
use probe::Probe;
use control::Control;
use structure::Structure;
use graph::Graph;
use graph::GraphSettings;
use preferences::Preferences;
use std::thread;
use serde_json;

pub struct Context<'a> {
    probes:      Vec<Box<Probe>>,
    control:     Control,
    graph:       Graph<'a>,
    structure:   Structure,
    preferences: Preferences,
    
}

impl<'a> Context<'a> {
    pub fn new(i : &Initial) -> Result<Context, String> {
        let mut control = Control::new().unwrap();
        let structure   = Structure::new();
        let graph       = Graph::new().unwrap();
        let preferences = Preferences::new();
        let s = serde_json::to_string(graph.settings).unwrap();
        println!("Result: {}", s);
        control.connect(|s| { println!("String: {:?}", s); Vec::from("rval")} );
        Ok(Context { probes: vec![],
                     control,
                     structure,
                     graph,
                     preferences })
    }

    pub fn run(&self) {
        loop {
            thread::sleep_ms(1000);
        }
    }
}
