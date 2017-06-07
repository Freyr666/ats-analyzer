#include "control.hpp"

using namespace Ats;

Control::Control () {
    in      = IOChannel::create_from_fd(0);
    out     = IOChannel::create_from_fd(1);
    out_log = IOChannel::create_from_fd(2);
    
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
    in->flush();
    received.emit(s);   
}

void
Control::send (const string& s) {
    out->write(s);
    out->write("\n");
    out->flush();
}

void
Control::error (const string& s) {
    out->write(s);
    out->write("\n");
    out->flush();
}

void
Control::log (const string& s) {
    string tmp = "Log: " + s;  
    out_log->write(s);
    out_log->write("\n");
    out_log->flush();
}
