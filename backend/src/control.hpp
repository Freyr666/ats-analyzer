#ifndef CONTROL_H
#define CONTROL_H

#include <gstreamermm.h>
#include <glibmm.h>
#include <exception>

#include "options.hpp"
#include "graph.hpp"

namespace Ats {

    enum class Msg_type {Json, Msgpack};

    class Control{

    public:
	class Wrong_msg : exception {};
	
    private:
	RefPtr<IOChannel> in;
	RefPtr<IOChannel> out;
	Msg_type          msg_type;
	
    public:
	Control (Msg_type t = Msg_type::Json);
	Control (const Control&) = delete;
	Control (Control&&) = delete;

	sigc::signal<void,const string&> control_received;
	sigc::signal<void,const int&>    options_received;

        void recv ();
	void send (const std::string&);
	
	void connect (const Options&);
	void connect (const Graph&);
    };

};

#endif /* CONTROL_H */
