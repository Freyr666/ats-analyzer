use std::thread;
use std::io::{self, Read};
use std::sync::{Arc,Mutex};
use std::sync::mpsc::*;
use std::str::FromStr;
use signals::Msg;
use zmq;

pub struct Control {
    ctx:        zmq::Context,
   // out_socket: zmq::Socket,
    pub sender:     Sender<Vec<u8>>,
    pub received: Arc<Mutex<Msg<Vec<u8>,Vec<u8>>>>
}

impl Control {
    
    pub fn new () -> Result<Control,String> {
        let ctx            = zmq::Context::new();
        let in_socket      = ctx.socket(zmq::REP).unwrap();
        let out_socket     = ctx.socket(zmq::PUB).unwrap();
        let (sender, receiver): (Sender<Vec<u8>>, Receiver<Vec<u8>>) = channel();
        let received       = Arc::new(Mutex::new(Msg::new()));
        
        in_socket.bind("ipc:///tmp/ats_qoe_in");
        out_socket.bind("ipc:///tmp/ats_qoe_out");
        let r = received.clone();
        
        thread::Builder::new().name("recv_thread".to_string()).spawn(move || {
            loop {
                let buffer = in_socket.recv_bytes(0).unwrap();
                let reply : Vec<u8> = r.lock().unwrap().emit(buffer).unwrap();
                in_socket.send (&reply, 0).unwrap();
               // reply.map(|buf| in_socket.send (&buf, 0));
            }
        }).unwrap();

        thread::Builder::new().name("send_thread".to_string()).spawn(move || {
            for msg in receiver {
                //println!("Msg: {}", String::from_utf8_lossy(&msg));
                out_socket.send(&msg,0).unwrap();
            }
        }).unwrap();

        Ok (Control { ctx, sender, received })
    }

   // pub fn send(&self, buf: &Vec<u8>) {
   //     self.out_socket.send(buf, 0);
   // }
    
    pub fn connect<F> (&mut self, f: F)
        where F: Fn(Vec<u8>) -> Vec<u8> + Send + Sync + 'static {
        self.received.lock().unwrap().connect(f).unwrap()
    }
}
