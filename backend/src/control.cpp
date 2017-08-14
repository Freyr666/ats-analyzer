#include "control.hpp"

using namespace Ats;
using namespace std;
using namespace Glib;

Control::Control () : context(1),
                      in_socket(context, ZMQ_REP),
                      out_socket(context, ZMQ_PUB) {

    out_log = IOChannel::create_from_fd(2);
    out_log->set_encoding("");

    /* NOTE: almost all cppzmq calls can throw an exception */

    in_socket.bind("ipc:///tmp/ats_qoe_in");
    out_socket.bind("ipc:///tmp/ats_qoe_out");

    auto read_in = [this](Glib::IOCondition c) -> bool {
        recv();
        return true;
    };

    int in_fd  = in_socket.getsockopt<int>(ZMQ_FD);
    in  = IOChannel::create_from_fd(in_fd);

    Glib::signal_io().connect(read_in, in, Glib::IO_IN);
}

void
Control::recv () {
    int ev = in_socket.getsockopt<int>(ZMQ_EVENTS);
    do {
        if (ev & ZMQ_POLLIN) {
            zmq::message_t m;
            try {
                if (in_socket.recv(&m)) {
                    uint8_t* mptr = static_cast<uint8_t*>(m.data());
                    std::vector<std::uint8_t> data(mptr, mptr + m.size());
                    std::string s = received.emit(data);
                    zmq::message_t reply (s.data(), s.length());
                    in_socket.send(reply);
                }
            } catch (const std::exception& e) {
                log(std::string("Exception while receiving a message: ") + e.what());
            }
        }
        else break;
        ev = in_socket.getsockopt<int>(ZMQ_EVENTS);

    } while (ev & ZMQ_POLLIN);
}

void
Control::send (const string& s) {
    zmq::message_t m(s.data(), s.length());
    out_socket.send(m);
}

void
Control::error (const string& s) {
    send(s); // FIXME?
}

void
Control::log (const string& s) {
    string tmp = "Log: " + s;  
    out_log->write(s);
    out_log->write("\n");
    out_log->flush();
}
