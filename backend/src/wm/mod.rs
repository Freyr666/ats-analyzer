pub mod position;
pub mod widget;
pub mod widget_video;
pub mod treeview;

use serde::Serialize;
use std::collections::HashMap;
use wm::widget::Widget;
use wm::treeview::Treeview;

pub struct WmState {
    widgets:    HashMap<String,Box<Widget>>,
    treeview:   Treeview,
    resolution: (u32, u32),
}
