use chatterer::ChattererProxy;
use chatterer::Logger;
use initial::Initial;
use probe::Probe;
use control::Control;
use streams::Streams;
use settings::Settings;

pub struct Context {
    probes: Vec<Box<Probe>>
}

impl Context {
    pub fn new(i : &Initial) -> Result<Context, String> {
        Ok(Context { probes: vec![] })
    }
}
