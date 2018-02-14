use std::collections::HashMap;
use wm::position::Position;
use wm::widget::WidgetDesc;

#[derive(Serialize,Deserialize,Clone)]
pub struct ContainerTemplate {
    pub position: Position,
    pub widgets:  Vec<(String,WidgetDesc)>,
}

#[derive(Serialize,Deserialize,Clone)]
pub struct WmTemplatePartial {
    resolution: (u32, u32),
    layout:     Vec<(String,ContainerTemplate)>,
}

#[derive(Serialize,Deserialize,Clone)]
pub struct WmTemplate {
    pub resolution: (u32, u32),
    pub layout:     Vec<(String,ContainerTemplate)>,
    pub widgets:    Vec<(String,WidgetDesc)>,
}

impl WmTemplate {

    pub fn from_partial (part: WmTemplatePartial, widgets: HashMap<String,WidgetDesc>) -> WmTemplate {
        let widgets = widgets.iter().map(|(name,w)| (name.clone(), w.clone())).collect();
        WmTemplate { resolution: part.resolution, layout: part.layout, widgets }
    }

    pub fn validate (&self) -> Result<(),String> {
        let res_pos      = Position::from_pair(self.resolution);
        for &(ref cname, ref c) in &self.layout {
            if ! c.position.is_in(&res_pos) {
                return Err(format!("container {}: is out of screen borders", cname))
            }
            for  &(ref wname, ref w) in &c.widgets {
                if ! self.widgets.iter().any(|&(ref name,_)| *name == *wname) {
                    return Err(format!("{}: no such widget", wname))
                }
                if ! w.position.is_in(&c.position) {
                    return Err(format!("{}: is out of container's borders", wname))
                }
                // not very optimal
                if c.widgets.iter().any(|&(ref name, ref wdg)| *name != *wname && wdg.position.is_overlap(&w.position)) {
                    return Err(format!("{}: intersects with another widget", wname))
                }
            }
        }
        Ok(())
    }
}
