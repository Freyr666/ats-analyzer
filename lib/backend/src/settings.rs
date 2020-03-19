use std::collections::HashMap;
use branch::Typ;

#[derive(Default,Copy,Clone,PartialEq,Serialize,Deserialize,Debug)]
pub struct Setting {
    pub peak_en : bool,
    pub peak    : f32,
    pub cont_en : bool,
    pub cont    : f32,
    pub duration: f32,
}

#[derive(Default,Copy,Clone,PartialEq,Serialize,Deserialize,Debug)]
pub struct Black {
    pub black       : Setting,
    pub luma        : Setting,
    pub black_pixel : u32,
}

#[derive(Default,Copy,Clone,PartialEq,Serialize,Deserialize,Debug)]
pub struct Freeze {
    pub freeze     : Setting,
    pub diff       : Setting,
    pub pixel_diff : u32,
}

#[derive(Default,Copy,Clone,PartialEq,Serialize,Deserialize,Debug)]
pub struct Blocky {
    pub blocky      : Setting,
}

#[derive(Default,Copy,Clone,PartialEq,Serialize,Deserialize,Debug)]
pub struct Silence {
    pub silence : Setting,
}

#[derive(Default,Copy,Clone,PartialEq,Serialize,Deserialize,Debug)]
pub struct Loudness {
    pub loudness : Setting,
}

#[derive(Default,Copy,Clone,PartialEq,Serialize,Deserialize,Debug)]
pub struct Adv {
    pub adv_diff : f32,
    pub adv_buf  : i32,
}

#[derive(Default,Copy,Clone,PartialEq,Serialize,Deserialize,Debug)]
pub struct Video {
    pub loss   : f32,
    pub black  : Black,
    pub freeze : Freeze,
    pub blocky : Blocky,
}

#[derive(Default,Copy,Clone,PartialEq,Serialize,Deserialize,Debug)]
pub struct Audio {
    pub loss     : f32,
    pub silence  : Silence,
    pub loudness : Loudness,
    pub adv      : Adv
}

#[derive(Clone,PartialEq,Serialize,Deserialize,Debug)]
pub struct SettingsFlat {
    default_video : Video,
    default_audio : Audio,
    video : Vec<(String,u32,Video)>,
    audio : Vec<(String,u32,Audio)>,
}

#[derive(Clone,PartialEq,Debug)]
pub struct Settings {
    default_video : Video,
    default_audio : Audio,
    video : HashMap<(String,u32), Video>, // TODO consider Cow<str>
    audio : HashMap<(String,u32), Audio>,
}
    

impl Settings {
    pub fn new () -> Settings { //default_video: Video, default_audio: Audio) -> Settings {
        let mut res = Settings { default_video : Video::default(),
                                 default_audio : Audio::default(),
                                 video : HashMap::new(),
                                 audio : HashMap::new(),
        };

        res
    }

    pub fn from_flat (fs : SettingsFlat) -> Settings {
        let mut video = HashMap::new();
        let mut audio = HashMap::new();
        for e in fs.video {
            video.insert((e.0, e.1), e.2);
        }
        for e in fs.audio {
            audio.insert((e.0, e.1), e.2);
        }
        Settings { video, audio,
                   default_video : fs.default_video,
                   default_audio : fs.default_audio,
        }
    }

    pub fn to_flat (&self) -> SettingsFlat {
        let video : Vec<_> = self.video
            .iter()
            .map(|((s,p),v)| (s.clone(),*p,*v))
            .collect();
        let audio : Vec<_> = self.audio
            .iter()
            .map(|((s,p),v)| (s.clone(),*p,*v))
            .collect();
        SettingsFlat { video, audio,
                       default_video : self.default_video,
                       default_audio : self.default_audio,
        }
    }
    
    pub fn get_video (&self, k: &(String,u32)) -> Video {
        match self.video.get(k) {
            Some (v) => *v,
            None => self.default_video,
        }
    }

    pub fn get_audio (&self, k: &(String,u32)) -> Audio {
        match self.audio.get(k) {
            Some (v) => *v,
            None => self.default_audio,
        }
    }

    pub fn set_video (&mut self, k: (String,u32), v: Video) {
        let _ = self.video.insert(k, v);
    }

    pub fn set_audio (&mut self, k: (String,u32), v: Audio) {
        let _ = self.audio.insert(k, v);
    }

    
}
