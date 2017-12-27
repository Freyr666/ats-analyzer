use std::thread;
use std::sync::Arc;
use std::sync::Mutex;
use std::io::{self, Read};
use signals::Msg;
use zmq;

pub struct Control {
    ctx:        zmq::Context,
    out_socket: zmq::Socket,
    received:   Arc<Mutex<Msg<Vec<u8>,Vec<u8>>>>
}

impl Control {
    
    pub fn new () -> Result<Control,String> {
        let ctx            = zmq::Context::new();
        let mut in_socket  = ctx.socket(zmq::REP).unwrap();
        let mut out_socket = ctx.socket(zmq::PUB).unwrap();
        let received       = Arc::new(Mutex::new(Msg::new()));
        
        in_socket.bind("ipc:///tmp/ats_qoe_in");
        out_socket.bind("ipc:///tmp/ats_qoe_out");
        let r         = received.clone();
        
        match thread::Builder::new().name("recv_thread".to_string()).spawn(move || {
            loop {
                let buffer = in_socket.recv_bytes(0).unwrap();
                let reply : Option<Vec<u8>> = r.lock().unwrap().emit(&buffer);
                reply.map(|buf| in_socket.send (&buf, 0));
            }
        }) {
            Ok(_)  => (),
            Err(_) => return Err(String::from("thread err"))
        }

        Ok (Control { ctx, out_socket, received })
    }

    pub fn send(&self, buf: &Vec<u8>) {
        self.out_socket.send(buf, 0);
    }
    
    pub fn connect (&mut self, f: fn(&Vec<u8>) -> Vec<u8>) {
        self.received.lock().unwrap().connect(f);
    }
}
