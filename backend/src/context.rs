//use chatterer::ChattererProxy;
//use chatterer::Logger;
use chatterer::Chatterer;
use initial::Initial;
use probe::Probe;
use control::Control;
use structure::Structure;
use graph::Graph;
use preferences::Preferences;
use std::thread;

pub struct Context {
    probes:      Vec<Box<Probe>>,
    control:     Control,
    graph:       Graph,
    structure:   Structure,
    preferences: Preferences,
    
}

impl Context {
    pub fn new(i : &Initial) -> Result<Context, String> {
        let mut control = Control::new().unwrap();
        let structure   = Structure::new();
        let graph       = Graph::new().unwrap();
        let preferences = Preferences::new();
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
