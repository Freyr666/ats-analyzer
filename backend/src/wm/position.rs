#[derive(Serialize,Deserialize,Clone,Copy,PartialEq,Debug)]
pub struct Position {
    pub left:   u32,
    pub top:    u32,
    pub right:  u32,
    pub bottom: u32,
}

impl Position {
    pub fn new () -> Position {
        Position { left: 0, top: 0, right: 0, bottom: 0 }
    }

    pub fn from_pair ((right,bottom): (u32,u32)) -> Position {
        Position { left: 0, top: 0, right, bottom }
    }
    pub fn get_height (&self) -> u32 { self.bottom - self.top }
    pub fn get_width (&self) -> u32 { self.right - self.left }
    pub fn get_x (&self) -> u32 { self.left }
    pub fn get_y (&self) -> u32 { self.top }

    pub fn is_in (&self, other: &Position) -> bool {
        self.top >= other.top
            && self.bottom <= other.bottom
            && self.left >= other.left
            && self.right <= other.right
    }

    pub fn is_overlap (&self, other: &Position) -> bool {
        self.left < other.right && self.right > other.left &&
            self.top > other.bottom && self.bottom < other.top
    }
}
