use std::collections::HashMap;
use wm::position::Position;
use wm::widget::WidgetDesc;

#[derive(Serialize,Deserialize,Clone,Debug)]
pub struct ContainerTemplate {
    pub position: Position,
    pub widgets:  Vec<(String,WidgetDesc)>,
}

#[derive(Serialize,Deserialize,Clone,Debug)]
pub struct WmTemplatePartial {
    resolution: (u32, u32),
    layout:     Vec<(String,ContainerTemplate)>,
}

#[derive(Serialize,Deserialize,Clone,Debug)]
pub struct WmTemplate {
    pub resolution: (u32, u32),
    pub layout:     Vec<(String,ContainerTemplate)>,
    pub widgets:    Vec<(String,WidgetDesc)>,
}

impl WmTemplate {

    pub fn from_partial (part: WmTemplatePartial) -> WmTemplate {
        let widgets = Vec::new();
        part.layout
            .iter()
            .map(|(_,cont)| widgets.append(&mut cont.widgets.clone()));
        WmTemplate { resolution: part.resolution, layout: part.layout, widgets }
    }

    pub fn validate (&self) -> Result<(),String> {
        for &(ref cname, ref c) in &self.layout {
            if ! c.position.validate() {
                return Err(format!("container {}: invalid normalized coordinates", cname))
            }
            for  &(ref wname, ref w) in &c.widgets {
                let position = w.position.unwrap_or(c.position);

                if ! self.widgets.iter().any(|&(ref name,_)| *name == *wname) {
                    return Err(format!("widget {}: no such widget", wname))
                }
                if ! position.validate() {
                    return Err(format!("widget {}: invalid normalized coordinates", wname))
                }
                // not very optimal
                if c.widgets.iter()
                    .any(|&(ref name, ref wdg)| {
                        let other_position = wdg.position.unwrap_or(c.position);
                        *name != *wname
                        && wdg.layer == w.layer
                        && other_position.is_overlapped(&position)}) {
                    return Err(format!("widget {}: intersects with another widget", wname))
                }
            }
        }
        Ok(())
    }
}
