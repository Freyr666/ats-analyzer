use gst;
use pad::{Type,SrcPad};
use signals::Signal;
use std::sync::{Arc,Mutex};
use wm::position::Position;
use wm::position::Absolute;

#[derive(Serialize,Deserialize,Clone,Debug)]
pub enum Domain {
    Chan { stream: String, channel: u32 },
    Nihil,
}

#[derive(Serialize,Deserialize,Clone,Debug)]
pub struct WidgetDesc {
    #[serde(rename = "type")]
    pub typ:         Type,
    pub domain:      Domain,
    pub pid:         Option<u32>,
    pub position:    Option<Position>,
    pub layer:       i32,
    pub aspect:      Option<(u32,u32)>,
    pub description: String,
}

pub trait Widget {
    fn add_to_pipe (&self, &gst::Bin);
    fn plug_src (&mut self, &SrcPad);
    fn plug_sink (&mut self, gst::Pad);
    fn gen_uid (&mut self) -> String;
    fn get_desc (&self) -> WidgetDesc;
    fn render (&mut self, &Absolute, Position, i32);
    fn disable (&mut self);
    fn linked (&self) -> Arc<Mutex<Signal<()>>>;
}
