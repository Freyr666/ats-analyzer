#[derive(Serialize,Deserialize,Clone,Copy,PartialEq,Debug)]
pub struct Position {
    pub x: f64,
    pub y: f64,
    pub w: f64,
    pub h: f64,
}

impl Position {
    pub fn new () -> Position {
        Position { x: 0.0, y: 0.0, w: 0.0, h: 0.0 }
    }

    pub fn from_pair ((w, h): (u32, u32)) -> Position {
        Position { x: 0.0, y: 0.0, w: w as f64, h: h as f64 }
    }
    pub fn get_height (&self) -> u32 { self.h.floor() as u32 }
    pub fn get_width (&self) -> u32 { self.w.floor() as u32 }
    pub fn get_x (&self) -> u32 { self.x.floor() as u32 }
    pub fn get_y (&self) -> u32 { self.y.floor() as u32 }

    fn right (&self) -> f64 { self.x + self.y }
    fn bottom (&self) -> f64 { self.y + self.h }

    pub fn is_in (&self, other: &Position) -> bool {
        self.y >= other.y
            && self.bottom() <= other.bottom()
            && self.x >= other.x
            && self.right() <= other.right()
    }

    pub fn is_overlapped (&self, other: &Position) -> bool {
        self.x < other.right()
            && self.right() > other.x
            && self.y < other.bottom()
            && self.bottom() > other.y
    }

    pub fn adjusted_by_left_upper (&self, other: &Position) -> Position {
        Position { x: self.x + other.x,
                   w: self.w,
                   y: self.y + other.y,
                   h: self.h }
    }

    pub fn to_absolute (&self, (w, h): (f64, f64)) -> Position {
        let w = self.w * w ;
        let h = self.h * h;
        Position { x: self.x * w / self.w,
                   y: self.y * h / self.h,
                   w,
                   h }
    }
}
