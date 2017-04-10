#ifndef CONTROL_H
#define CONTROL_H

#include <gstreamermm.h>
#include <glibmm.h>
#include <exception>

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
        RefPtr<IOChannel> out;
        RefPtr<IOChannel> out_log;
        Msg_type          msg_type;
	
    public:
        Control (Msg_type t = Msg_type::Debug);
        Control (const Control&) = delete;
        Control (Control&&) = delete;

        sigc::signal<void,const string&> received_json;
        sigc::signal<void,const string&> received_msgpack;

	void set_msg_type(Msg_type t) { msg_type = t; }
        void recv ();
        void send (const Chatterer&);
        void error(const std::string&);
        void log  (const std::string&);
        
        void   connect(Chatterer& c) {
            c.send.connect(sigc::mem_fun(this, &Control::send));
            c.send_err.connect(sigc::mem_fun(this, &Control::error));
            c.send_log.connect(sigc::mem_fun(this, &Control::log));
        }
    };

};

#endif /* CONTROL_H */
