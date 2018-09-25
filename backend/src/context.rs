//use chatterer::ChattererProxy;
//use chatterer::Logger;
use initial::Initial;
use settings::Configuration;
use probe::Probe;
use control::Control;
use streams::{Streams};
use graph::Graph;
use std::boxed::Box;
use std::collections::HashMap;
use gst;
use glib;
use std::sync::{Arc,Mutex};

use chatterer::MsgType;
use chatterer::control::{Addressable,Dispatcher,DispatchTable};
use chatterer::notif::Notifier;

#[derive(Serialize)]
enum Status { Ready }

pub struct ContextDispatcher {
    format:      MsgType,
    table:       HashMap<String, (Box<Fn(Vec<u8>) -> Vec<u8> + Send + Sync>)>,
}

pub struct Context {
    mainloop:    glib::MainLoop,
    notif:       Notifier,
}

impl Addressable for ContextDispatcher {
    fn get_name (&self) -> &str { "backend" }
    fn get_format (&self) -> MsgType { self.format }
}

impl DispatchTable for ContextDispatcher {
    fn add_respondent (&mut self, s: String, c: Box<Fn(Vec<u8>) -> Vec<u8> + Send + Sync>) {
        self.table.insert(s, c);
    }
    
    fn get_respondent (&self, s: &str) -> Option<&Box<Fn(Vec<u8>) -> Vec<u8> + Send + Sync>> {
        self.table.get(s)
    }
}

impl Dispatcher for ContextDispatcher {}

impl ContextDispatcher {
    pub fn new (format: MsgType) -> ContextDispatcher {
        let table       = HashMap::new();
        ContextDispatcher { table, format }
    }
}

impl Context {
    pub fn new (i : &Initial) -> Result<Context, String> {
        gst::init().unwrap();

        let mainloop    = glib::MainLoop::new(None, false);
        let mut control = Control::new().unwrap();

        let dispatcher  = Arc::new(Mutex::new(ContextDispatcher::new(i.msg_type)));
        let notif       = Notifier::new("backend", i.msg_type, control.sender.clone());
        
        let mut probes  = Vec::new();

        for sid in 0..i.uris.len() {
            probes.push(Probe::new(sid as i32,&i.uris[sid]));
        };

        let     config  = Configuration::new(i.msg_type, control.sender.clone());
        let mut streams = Streams::new(i.msg_type, control.sender.clone());
        let mut graph   = Graph::new(i.msg_type, control.sender.clone()).unwrap();
        
        for probe in &mut probes {
            probe.set_state(gst::State::Playing);
            streams.connect_probe(probe);
        }
        
        dispatcher.lock().unwrap().add_to_table(&config);
        dispatcher.lock().unwrap().add_to_table(&streams);
        dispatcher.lock().unwrap().add_to_table(&graph);
        let wm = graph.get_wm();
        dispatcher.lock().unwrap().add_to_table(&(*wm.lock().unwrap()));

        let dis = dispatcher.clone();
        control.connect(move |s| {
            // debug!("Control message received");
            dis.lock().unwrap().dispatch(s).unwrap()
            // debug!("Control message response ready");
        });

        graph.connect_destructive(&mut streams.update.lock().unwrap());
        graph.connect_settings(&mut config.update.lock().unwrap());
        
        info!("Context was created");
        Ok(Context { mainloop, notif })
    }

    pub fn run(&self) {
        // TODO fi this hack
        // thread::sleep(time::Duration::from_millis(500)); // a very dirty hack indeed
        self.notif.talk(&Status::Ready);
        self.mainloop.run();
    }
}
