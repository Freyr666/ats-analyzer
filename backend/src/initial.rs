use std::env::Args;

#[derive(Debug)]
pub enum Error {
    HelpOption,
    WrongOption (String)
}

#[derive(Debug)]
pub enum MsgType {
    Json,
    Msgpack,
    Debug
}

#[derive(Debug)]
pub struct Initial {
    pub uris:              Vec<String>,
    pub multicast_address: Option<String>,
    pub msg_type:          Option<MsgType>
}

impl Initial {    
    pub fn new(args : &Args) -> Result<Initial, Error> {
        /* Options:
         * -h help
         * -m msgtype
         * -o output uri
         */
        let mut ma : Option<String> = None;
        let mut mt : Option<MsgType>  = Some(MsgType::Json);
        let uris : Vec<String> = vec!["udp://224.1.2.2:1234".to_string()];

        Ok ( Initial { uris, multicast_address : ma, msg_type : mt } )
    }

    pub fn usage() -> &'static str {
        "Usage:\n\
         [-opt arg] uri1 [uri2 uri3]\n\
         Options:\n\
         \t-o\toutput uri\n\
         \t-m\tipc message type [json | msgpack | debug]\n\
         \t-h\thelp\n\
         Additional:\n\
         \turi format: udp://[ip] or udp://[ip]:[port]\n"
    }
}
