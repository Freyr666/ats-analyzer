use std::sync::mpsc::*;
use std::thread;

pub struct Callbacks<T> {
    pub process: Box<Fn(&T) + Send + Sync + 'static>,
    pub thread_reg: Box<Fn() + Send + Sync + 'static>,
    pub thread_unreg: Box<Fn() + Send + Sync + 'static>,
}

pub fn create <T> (cb: Callbacks<T>) -> Sender<T>
where T: Send + 'static {

    let (sender, receiver): (Sender<T>, Receiver<T>) = channel();

    thread::spawn(move || {
        (cb.thread_reg)();
        //println!("Started, {}", name);
        for msg in receiver {
            (cb.process)(&msg);
        }
        //println!("Finished {}", name);
        (cb.thread_unreg)();
    });

    sender
}
