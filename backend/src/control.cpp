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
    in->read_to_end(s);
}

void
Control::send (const std::string& s) {
    out->write(s);
}
