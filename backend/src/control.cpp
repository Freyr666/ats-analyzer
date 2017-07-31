#include "control.hpp"

using namespace Ats;

Control::Control () : context(1),
                      in_socket(context, ZMQ_SUB),
                      out_socket(context, ZMQ_PUB) {

    out_log = IOChannel::create_from_fd(2);
    out_log->set_encoding("");

    /* NOTE: almost all cppzmq calls can throw an exception */

    in_socket.connect("ipc:///tmp/ats_qoe_in");
    out_socket.bind("ipc:///tmp/ats_qoe_out");

    // temp
    const char *filter = "";
    in_socket.setsockopt(ZMQ_SUBSCRIBE, filter, strlen (filter));

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
                if (in_socket.recv(&m) && (m.size() > 0)) {
                    uint8_t* mptr = static_cast<uint8_t*>(m.data());
                    std::vector<std::uint8_t> data(mptr, mptr + m.size());
                    received.emit(data);

                    // NOTE: maybe need to do smth if the message is empty?
                    send(string(data.begin(), data.end()));
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
    cout << s << endl;
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
