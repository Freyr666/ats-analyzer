
#[derive(Clone,PartialEq,Serialize,Deserialize,Debug)]
pub struct VideoPid {
    pub resolution:   (i32,i32),
    pub aspect_ratio: (i32,i32),
    pub frame_rate:   f32,
    pub codec:        String,
    pub interlaced:   String
}

#[derive(Clone,PartialEq,Serialize,Deserialize,Debug)]
pub struct AudioPid {
    pub channels:     u32,
    pub sample_rate:  u32,
    pub codec:        String,
    pub bitrate:      String
}

#[derive(Clone,PartialEq,Serialize,Deserialize,Debug)]
pub enum PidContent {
    Video (VideoPid),
    Audio (AudioPid),
    Empty
}

#[derive(Clone,PartialEq,Serialize,Deserialize,Debug)]
pub struct Pid {
    pub pid:              u32,
    pub to_be_analyzed:   bool,
    pub content:          PidContent,
    pub stream_type:      u32,
    pub stream_type_name: String
}

#[derive(Clone,PartialEq,Serialize,Deserialize,Debug)]
pub struct Channel {
    pub number:        u32,
    pub service_name:  String,
    pub provider_name: String,
    pub pids:          Vec<Pid>
}

#[derive(Clone,PartialEq,Serialize,Deserialize,Debug)]
pub struct Structure {
    pub id:       i32, //[@key "stream"] (* TODO replace by id and u32 *)
    pub uri:      String,
    pub channels: Vec<Channel>
}

impl Pid {
    pub fn new (pid: u32, stream_type: u32, stream_type_name: String) -> Pid {
        // TODO init pid_content
        Pid { pid, to_be_analyzed: false, content: PidContent::Empty, stream_type, stream_type_name }
    }
}

impl Channel {
    pub fn new (number: u32, service_name:  String, provider_name: String) -> Channel {
        Channel { number, service_name, provider_name, pids: vec![] }
    }

    pub fn new_empty (number: u32) -> Channel {
        Channel { number, service_name: String::new(), provider_name: String::new(), pids: vec![] }
    }

    pub fn append_pid (&mut self, p: Pid) {
        self.pids.push(p)
    }

    pub fn pids_num (&self) -> usize {
        self.pids.len()
    }

    pub fn to_be_analyzed (&self) -> bool {
        self.pids.iter().any(|p| p.to_be_analyzed)
    }

    pub fn pid_exists (&self, pid: u32) -> bool {
        self.pids.iter().any(|p| p.pid == pid)
    }

    pub fn find_pid (&self, pid: u32) -> Option<&Pid> {
        self.pids.iter().find(|p| p.pid == pid)
    }

    pub fn find_pid_mut (&mut self, pid: u32) -> Option<&mut Pid> {
        self.pids.iter_mut().find(|p| p.pid == pid)
    }
}

impl Structure {
    pub fn new (uri: String, id: i32) -> Structure {
        Structure { id, uri, channels: vec![] }
    }

    pub fn from (&mut self, other: &Structure) {
        self.channels = other.channels.clone()
    }
    
    pub fn add_channel (&mut self, c: Channel) {
        self.channels.push(c)
    }

    pub fn is_empty (&self) -> bool {
        self.channels.is_empty()
    }

    pub fn channels_num (&self) -> usize {
        self.channels.len()
    }

    pub fn to_be_analyzed (&self) -> bool {
        self.channels.iter().any(|c| c.to_be_analyzed())
    }
    
    pub fn clear (&mut self) {
        self.channels.clear()
    }

    pub fn channel_exists (&self, num: u32) -> bool {
        self.channels.iter().any(|c| c.number == num )
    }
    
    pub fn find_channel (&self, num: u32) -> Option<&Channel> {
        self.channels.iter().find(|c| c.number == num )
    }

    pub fn find_channel_mut (&mut self, num: u32) -> Option<&mut Channel> {
        self.channels.iter_mut().find(|c| c.number == num )
    }

    pub fn find_channel_mut_unsafe (&mut self, num: u32) -> &mut Channel {
        self.channels.iter_mut().find(|c| c.number == num ).unwrap()
    }

    pub fn find_pid (&self, num: u32, pid: u32) -> Option<&Pid> {
        let c = self.find_channel(num)?;
        c.find_pid(pid)
    }

    pub fn find_pid_mut (&mut self, num: u32, pid: u32) -> Option<&mut Pid> {
        let c = self.find_channel_mut(num)?;
        c.find_pid_mut(pid)
    }

    pub fn find_pid_by_num (&self, pid: u32) -> Option<&Pid> {
        for c in &self.channels {
            if let Some(p) = c.find_pid(pid) {
                return Some(p)
            }
        }
        None
    }

    pub fn find_pid_by_num_mut (&mut self, pid: u32) -> Option<&mut Pid> {
        for c in &mut self.channels {
            if let Some(p) = c.find_pid_mut(pid) {
                return Some(p)
            }
        }
        None
    }    
}
