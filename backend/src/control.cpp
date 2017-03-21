#include "control.hpp"

using namespace Ats;

Control::Control (Msg_type t) : msg_type(t) {
    in  = IOChannel::create_from_fd(0);
    out = IOChannel::create_from_fd(1);
    
    const auto read_in = [this](Glib::IOCondition c) -> bool {
        recv();
        return true;
    };

    Glib::signal_io().connect(read_in,
                              in, Glib::IO_IN);
	
}

void
Control::recv () {
    Glib::ustring s;
    in->read_line(s);
    switch (msg_type) {
    case Msg_type::Debug:
    case Msg_type::Json: 
        received_json.emit(s);    break;
    case Msg_type::Msgpack:
        received_msgpack.emit(s); break;
    }
}

void
Control::send (const Chatterer& c) {
    string s;
    switch (msg_type) {
    case Msg_type::Debug:
        s = c.to_string();  break;
    case Msg_type::Json:
        s = c.to_json();    break;
    case Msg_type::Msgpack:
        s = c.to_msgpack(); break;
    }
    out->write(s);
    out->flush();
}
