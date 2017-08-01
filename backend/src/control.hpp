#ifndef CONTROL_H
#define CONTROL_H

#include <gstreamermm.h>
#include <glibmm.h>
#include <exception>
#include <zmq.hpp>

#include "options.hpp"
// #include "graph.hpp"
#include "chatterer.hpp"
#include "msgtype.hpp"

using namespace std;

namespace Ats {

    class Control{

    public:
        class Wrong_msg : exception {};
	
    private:
        RefPtr<IOChannel> in;
        RefPtr<IOChannel> out_log;

        zmq::context_t context;
        zmq::socket_t  in_socket;
        zmq::socket_t  out_socket;

    public:
        Control ();
        Control (const Control&) = delete;
        Control (Control&&) = delete;

        sigc::signal<std::string,const vector<std::uint8_t>&> received;

        void init ();
        void recv ();
        void send (const std::string&);
        void error(const std::string&);
        void log  (const std::string&);
        
        void   connect(Chatterer_proxy& c) {
            c.send.connect(sigc::mem_fun(this, &Control::send));
            c.send_err.connect(sigc::mem_fun(this, &Control::error));
        }
        void   connect(Logger& c) {
            c.send_log.connect(sigc::mem_fun(this, &Control::log));
        }
    };

};

#endif /* CONTROL_H */
