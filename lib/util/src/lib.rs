pub mod signals;
pub mod channels;
pub mod initial;

#[macro_export]
macro_rules! reraise {
    ($e:expr, $er:expr) => {{
        let estr : &'static str = $er;
        match $e {
            Ok(v) => v,
            Err(_) => return Err(estr.to_string())
        }
    }};
}

#[macro_export]
macro_rules! optraise {
    ($e:expr, $er:expr) => {{
        let estr : &'static str = $er;
        match $e {
            Some(v) => v,
            None => return Err(estr.to_string())
        }
    }};
}
