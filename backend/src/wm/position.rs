#[derive(Serialize,Deserialize,Clone,Copy,PartialEq,Debug)]
pub struct Position {
    pub x: f64,
    pub y: f64,
    pub w: f64,
    pub h: f64,
}

pub struct Absolute {
    pub left: u32,
    pub top: u32,
    pub width: u32,
    pub height: u32
}

impl Absolute {
    pub fn adjust_aspect (&self, aspect: (u32, u32)) -> Absolute {
        let frame_width = (self.width as f64);
        let frame_height = (self.height as f64);
        let frame_dar = frame_width / frame_height;
        let content_dar = (aspect.0 as f64) / (aspect.1 as f64);
        if frame_dar < content_dar {
            // letterbox
            let height = ((frame_height * frame_dar / content_dar) as u32);
            let top = self.top + (self.height - height) / 2;
            Absolute { height, top, ..*self }
        }
        else {
            // pillarbox
            let width = ((frame_width * content_dar / frame_dar) as u32);
            let left = self.left + (self.width - width) / 2;
            Absolute { width, left, ..*self }
        }
    }
}

impl Position {
    pub fn new () -> Position {
        Position { x: 0.0, y: 0.0, w: 0.0, h: 0.0 }
    }

    pub fn from_pair ((w, h): (f64, f64)) -> Position {
        Position { x: 0.0, y: 0.0, w, h }
    }

    fn right (&self) -> f64 { self.x + self.w }
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

    pub fn validate (&self) -> bool {
        self.is_in(&Position::from_pair((1.0, 1.0)))
    }

    pub fn to_absolute (&self, (w, h): (u32, u32)) -> Absolute {
        let width = self.w * (w as f64);
        let height = self.h * (h as f64);
        let left = self.x * width / self.w;
        let top = self.y * height / self.h;
        Absolute { left: left.floor() as u32,
                   top: top.floor() as u32,
                   width: width.floor() as u32,
                   height: height.floor() as u32 }
    }
}
