use std::collections::HashMap;

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
    pub id:               u32,
    pub group_id:         Option<u32>,
    pub service_name:     String,
    pub provider_name:    String,
    pub content:          PidContent,
    pub stream_type:      u32,
    pub stream_type_name: String
}

#[derive(Clone,PartialEq,Serialize,Deserialize,Debug)]
pub struct Structure {
    pub id:       String,
    pub uri:      String,
    pub pids:     Vec<Pid>
}

impl Pid {
    pub fn new (id: u32,
                group_id: Option<u32>,
                service_name:  String,
                provider_name: String,
                stream_type: u32,
                stream_type_name: String) -> Pid {
        // TODO init pid_content
        Pid { id,
              group_id,
              service_name,
              provider_name,
              content: PidContent::Empty,
              stream_type,
              stream_type_name }
    }
}

/*
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
*/

impl Structure {
    pub fn new (uri: String, id: String) -> Structure {
        Structure { id, uri, pids: vec![] }
    }
    /*
    pub fn from (&mut self, other: &Structure) {
        self.channels = other.channels.clone()
    }
     */

    pub fn add_pid (&mut self, c: Pid) {
        self.pids.push(c)
    }

    pub fn is_empty (&self) -> bool {
        self.pids.is_empty()
    }

    pub fn clear (&mut self) {
        self.pids.clear()
    }

    pub fn find_pid (&self, id: u32, group_id: Option<u32>) -> Option<&Pid> {
        self.pids.iter().find(|p| { p.id == id && p.group_id == group_id })
    }

    pub fn find_pid_mut (&mut self, id: u32, group_id: Option<u32>) -> Option<&mut Pid> {
        self.pids.iter_mut().find(|p| { p.id == id && p.group_id == group_id })
    }

    pub fn find_pid_by_num (&self, id: u32) -> Option<&Pid> {
        self.pids.iter().find(|p| { p.id == id })
    }

    pub fn find_pid_by_num_mut (&mut self, id: u32) -> Option<&mut Pid> {
        self.pids.iter_mut().find(|p| { p.id == id })
    }

    pub fn map_group_mut<F> (&mut self, group_id: Option<u32>, f: F)
    where F: Fn(&mut Pid) {
        for ref mut p in self.pids.iter_mut() {
            if p.group_id == group_id {
                f(p);
            }
        }
    }

    pub fn by_channel (&self) -> Vec<(u32, Vec<Pid>)> {
        let mut ch : HashMap<u32,Vec<Pid>> = HashMap::new();

        for p in &self.pids {
            match p.group_id {
                None => (),
                Some(gid) => {
                    /* No such key */
                    if let Some(v) = ch.get_mut (&gid) {
                        v.push (p.clone());
                        continue;
                    }
                    /* Otherwise */
                    ch.insert (gid, vec![p.clone()]);
                }
            }
        };

        ch.into_iter().collect()
    }
}
