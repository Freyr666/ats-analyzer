use std::sync::{Arc,Mutex};
use wm::widget::Widget;
use wm::widget_video::WidgetVideo;
use wm::widget_soundbar::WidgetSoundbar;

pub fn make (typ: &str) -> Option<Arc<Mutex<Widget + Send>>> {
    match typ {
        "video" => Some(Arc::new(Mutex::new(WidgetVideo::new()))),
        "audio" => Some(Arc::new(Mutex::new(WidgetSoundbar::new()))),
        _       => None
    }
}
