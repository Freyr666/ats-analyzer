extern crate backend;

use backend::initial::{Initial,Error};
use backend::context::Context;

fn main() {
    let args = std::env::args();
    let i    = match Initial::new(args) {
        Ok (i) => i,
        Err (Error::HelpOption) => {
            println!("{}",Initial::usage());
            std::process::exit(-1);
        },
        Err (Error::WrongOption(s)) => {
            println!("{}",s);
            std::process::exit(-1);
        }
    };
    let c    = Context::new(&i).unwrap();
    c.run()
}
