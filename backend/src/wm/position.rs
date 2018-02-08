#[derive(Serialize,Deserialize,Clone,Copy,PartialEq)]
pub struct Position {
    left:   u32,
    top:    u32,
    right:  u32,
    bottom: u32,
}

impl Position {
    pub fn new () -> Position {
        Position { left: 0, top: 0, right: 0, bottom: 0 }
    }

    pub fn get_height(&self) -> u32 { self.bottom - self.top }
    pub fn get_width(&self) -> u32 { self.right - self.left }
    pub fn get_x(&self) -> u32 { self.left }
    pub fn get_y(&self) -> u32 { self.top }
}
