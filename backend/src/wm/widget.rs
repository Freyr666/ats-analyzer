use gst;
use pad::SrcPad;
use wm::position::Position;

#[derive(Serialize,Deserialize,Clone)]
pub struct WidgetDesc {
    pub typ:         String,
    pub position:    Position,
    pub layer:       i32,
    pub aspect:      (u32,u32),
    pub description: String,
    pub enabled:     bool,
}

pub trait Widget {
    fn add_to_pipe(&self, gst::Bin);
    fn plug_src(&mut self, &SrcPad);
    fn plug_sink(&mut self, gst::Pad);
    fn gen_uid(&mut self) -> String;
    fn get_desc(&self) -> &WidgetDesc;
    fn apply_desc(&mut self, WidgetDesc);
}
