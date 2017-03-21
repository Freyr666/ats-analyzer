#ifndef CONTROL_H
#define CONTROL_H

#include <gstreamermm.h>
#include <glibmm.h>
#include <exception>

#include "options.hpp"
#include "graph.hpp"
#include "chatterer.hpp"

using namespace std;

namespace Ats {

    // Msg format types. Debug = of_json/to_string
    enum class Msg_type {Json, Msgpack, Debug};

    class Control{

    public:
	class Wrong_msg : exception {};
	
    private:
	RefPtr<IOChannel> in;
	RefPtr<IOChannel> out;
	Msg_type          msg_type;
	
    public:
	Control (Msg_type t = Msg_type::Debug);
	Control (const Control&) = delete;
	Control (Control&&) = delete;

	sigc::signal<void,const string&> received_json;
	sigc::signal<void,const string&> received_msgpack;

        void recv ();
	void send (const Chatterer&);

	void   connect(Chatterer& c) {
	    c.send.connect(sigc::mem_fun(this, &Control::send));
	}
    };

};

#endif /* CONTROL_H */
