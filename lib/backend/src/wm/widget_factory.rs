use std::sync::{Arc,Mutex};
use wm::widget::{Widget,WidgetDesc};
use wm::widget_video::WidgetVideo;
use wm::widget_soundbar::WidgetSoundbar;

pub fn from_typ (typ: &str) -> Result<Arc<Mutex<Widget + Send>>,String> {
    match typ {
        "video" => match WidgetVideo::new() {
            Err(e) => Err(e),
            Ok(w) => Ok(Arc::new(Mutex::new(w))),
        },
        "audio" => match WidgetSoundbar::new() {
            Err(e) => Err(e),
            Ok(w) => Ok(Arc::new(Mutex::new(w))),
        },
        _       => Err(format!("Widget: could not create a widget of type {}", typ)),
    }
}

pub fn from_desc (desc: &WidgetDesc) -> Result<Arc<Mutex<Widget + Send>>,String> {
    match desc.typ {
        "video" => match WidgetVideo::new() {
            Err(e) => Err(e),
            Ok(w) => Ok(Arc::new(Mutex::new(w))),
        },
        "audio" => match WidgetSoundbar::new() {
            Err(e) => Err(e),
            Ok(w) => Ok(Arc::new(Mutex::new(w))),
        },
        _       => Err(format!("Widget: could not create a widget of type {}", typ)),
    }
}
