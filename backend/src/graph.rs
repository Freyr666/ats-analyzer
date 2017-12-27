
pub struct Graph {
    x: i32
}

impl Graph {
    pub fn new() -> Result<Graph,String> {
        Ok(Graph { x: 42 })
    }
}
