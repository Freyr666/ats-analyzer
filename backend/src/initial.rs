use std::env::Args;
use chatterer::MsgType;

#[derive(Debug)]
pub enum Error {
    HelpOption,
    WrongOption (String)
}

pub type Uri = String;
pub type Id  = String;

#[derive(Debug)]
pub struct Initial {
    pub uris:     Vec<(Id,Uri)>,
    pub msg_type: MsgType
}

pub enum UriParserState {
    Ip,
    Port
}

pub enum ArgsParserState {
    MsgType,
    Uri(String),
    Positional
}

pub enum UriError {
    Prefix,
    OctetValue(i32,i32),
    OctetMissing(i32),
    OctetNumber(i32),
    PortValue(i32),
    PortMissing,
    BadSymbol(char),
}

pub fn uri_error_to_string(e : &UriError) -> String {
    match *e {
        UriError::Prefix                  => String::from("Bad prefix"),
        UriError::OctetValue(ref v,ref o) => format!("bad value ({}) in ip octet = {}", v, o + 1),
        UriError::OctetMissing(ref o)     => format!("ip octet {} is missing", o + 1),
        UriError::OctetNumber (ref o)     => format!("bad number ({}) of ip octets", o + 1),
        UriError::BadSymbol(ref s)        => format!("bad symbol: {}",s),
        UriError::PortValue(ref v)        => format!("bad port value ({})",v),
        UriError::PortMissing             => String::from("port value missing after semicolon"),
    }
}

pub fn check_ip_octet(v : i32, octet : i32) -> Result<i32, UriError> {
    if octet > 3             { Err (UriError::OctetNumber(octet)) }
    else if v < 0 || v > 255 { Err (UriError::OctetValue(v,octet)) }
    else                     { Ok (v) }
}

pub fn validate_uri(uri : &str) -> Result<&str, UriError> {

    let prefix = "udp://";
    if !uri.starts_with(prefix) { return Err (UriError::Prefix) }

    let trimmed = uri.trim_left_matches(prefix);
    let mut state   : UriParserState = UriParserState::Ip;
    let mut sym_cnt : u32 = 0;
    let mut val     : i32 = 0;
    let mut octet   : i32 = 0;
    for sym in trimmed.chars() {
        if sym >= '0' && sym <= '9' {
            val     = (val * 10) + ((sym.to_digit(10).unwrap()) as i32);
            sym_cnt += 1;
        }
        else if sym == '.' {
            match state {
                UriParserState::Ip =>
                { if sym_cnt == 0 { return Err (UriError::BadSymbol(sym)) };
                  match check_ip_octet(val,octet) {
                      Ok (_)  => (),
                      Err (e) => return Err (e),
                  };
                  octet   += 1;
                  val     = 0;
                  sym_cnt = 0;
                },
                UriParserState::Port => return Err (UriError::BadSymbol(sym)),
            }
        }
        else if sym == ':' {
            match state {
                UriParserState::Ip =>
                { if sym_cnt == 0    { return Err (UriError::BadSymbol(sym)) }
                  else if octet != 3 { return Err (UriError::OctetNumber(octet)) }
                  else {
                      match check_ip_octet(val,octet) {
                          Ok (_)  => (),
                          Err (e) => return Err (e),
                      };
                      val     = 0;
                      sym_cnt = 0;
                      state   = UriParserState::Port;
                  }
                },
                UriParserState::Port => { return Err (UriError::BadSymbol(sym)) },
            }
        }
        else { return Err (UriError::BadSymbol(sym)) }
    };
    match state {
        UriParserState::Ip   =>
        { if sym_cnt == 0    { Err (UriError::OctetMissing(octet)) }
          else if octet != 3 { Err (UriError::OctetNumber(octet)) }
          else {
              match check_ip_octet(val,octet) {
                  Ok (_)  => Ok (uri),
                  Err (e) => Err (e),
              }
          }
        }
        UriParserState::Port =>
        { if sym_cnt == 0                { Err (UriError::PortMissing) }
          else if val < 0 || val > 65535 { Err (UriError::PortValue(val)) }
          else                           { Ok (uri) }
        },
    }
}

impl Initial {

    pub fn new(args : Args) -> Result<Initial, Error> {
        /* Options:
         * -h help
         * -m msgtype
         */
        let mut mt    : MsgType         = MsgType::Json;
        let mut uris                    = Vec::new();
        let mut state : ArgsParserState = ArgsParserState::Positional;
        let mut args  : Vec<String>     = args.collect();
        let path      : String          = args.remove(0);
        for arg in args {
            match state {
                ArgsParserState::Positional =>
                { if arg.starts_with('-') {
                    match arg.as_str() {
                        "-h" | "--help"    => return Err (Error::HelpOption),
                        "-m" | "--msgtype" => state = ArgsParserState::MsgType,
                        x                  =>
                        { let s = format!("{}: unrecognized option '{}'", path, x);
                          return Err (Error::WrongOption(s));
                        }
                    };
                } else {
                    state = ArgsParserState::Uri(arg);
                }
                },
                ArgsParserState::Uri(id) => {
                    state = ArgsParserState::Positional;
                    match validate_uri(&arg) {
                        Ok (uri) => uris.push((id.clone(), String::from(uri))),
                        Err (e)  =>
                        { let e_str = uri_error_to_string(&e);
                          let s     = format!("{}: bad uri argument ({}), {}", path, arg, e_str);
                          return Err (Error::WrongOption(s))
                        }
                    }
                },
                ArgsParserState::MsgType => {
                    state = ArgsParserState::Positional;
                    match arg.as_str() {
                        "json"    => mt = MsgType::Json,
                        "msgpack" => mt = MsgType::Msgpack,
                        _         =>
                        { let s = format!("{}: bad msgtype value ({})", path, arg);
                          return Err (Error::WrongOption(s))
                        }
                    }
                },
            }
        };

        match state {
            ArgsParserState::Uri(id) => {
                let s = format!("{}: no uri argument for stream {}", path, id);
                Err (Error::WrongOption(s))
            },
            _ => Ok ( Initial { uris, msg_type : mt } )
        }
    }

    pub fn usage() -> &'static str {
        "Usage:\n\
         [-opt arg] id1 uri1 [id2 uri2 id3 uri3]\n\
         Options:\n\
         \t-m,\t--msgtype\tipc message type [json | msgpack]\n\
         \t-h,\t--help   \thelp\n\
         Additional:\n\
         \turi format: udp://[ip] or udp://[ip]:[port]\n"
    }
}
