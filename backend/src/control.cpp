#include "control.hpp"

using namespace Ats;

Control::Control () {
    in  = IOChannel::create_from_fd(0);
    out = IOChannel::create_from_fd(1);

    const auto read_in = [this](Glib::IOCondition c) -> bool {
        msg_recieved.emit(recv_msg());
	return true;
    };

    Glib::signal_io().connect(read_in,
			      in, Glib::IO_IN);
	
}

string
Control::recv_msg () {
    Glib::ustring s;
    in->read_to_end(s);
    return s;
}

void
Control::send_msg (const std::string& s) {
    out->write(s);
}
