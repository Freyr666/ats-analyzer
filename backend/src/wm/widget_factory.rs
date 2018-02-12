use std::sync::{Arc,Mutex};
use wm::widget::Widget;
use wm::widget_video::WidgetVideo;

pub fn make (typ: &str) -> Option<Arc<Mutex<Widget + Send>>> {
    match typ {
        "video" => Some(Arc::new(Mutex::new(WidgetVideo::new()))),
        _       => None
    }
}
