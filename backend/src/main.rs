#[macro_use]
extern crate log;
extern crate backend;

use log::{LevelFilter, Log, Record, Metadata};

use backend::initial::{Initial,Error};
use backend::context::Context;

#[cfg(feature = "std")]
use log::set_boxed_logger;

#[cfg(not(feature = "std"))]
fn set_boxed_logger(logger: Box<Log>) -> Result<(), log::SetLoggerError> {
    log::set_logger(unsafe { &*Box::into_raw(logger) })
}

struct Logger { name: String }

impl log::Log for Logger {
    // TODO remove metadata
    fn enabled(&self, _metadata: &Metadata) -> bool {
        // metadata.level() <= Level::Info
        true
    }

    fn log(&self, record: &Record) {
        if self.enabled(record.metadata()) {
            println!("{}: {} - {}", self.name, record.level(), record.args());
        }
    }

    fn flush(&self) {}
}

fn main() {
    let args = std::env::args();
    let log_level_filter = match std::env::var("ATS3_LOG_LEVEL") {
        Err (_) => LevelFilter::Off,
        Ok(ref s) => match s.as_str() {
            "error" => LevelFilter::Error,
            "warn"  => LevelFilter::Warn,
            "info"  => LevelFilter::Info,
            "debug" => LevelFilter::Debug,
            _ => LevelFilter::Off,
        },
    };
    
    set_boxed_logger(Box::new(Logger{ name: String::from("ats3-backend") })).unwrap();
    log::set_max_level(log_level_filter);
    
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
