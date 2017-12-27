
#[derive(Serialize, Deserialize, Debug)]
pub struct Chatterer {
    x: i32
}

impl Chatterer {
    pub fn new () -> Chatterer {
        Chatterer { x : 42 }
    }
}
