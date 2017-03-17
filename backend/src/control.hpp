#ifndef CONTROL_H
#define CONTROL_H

#include <gstreamermm.h>
#include <glibmm.h>

#include "options.hpp"
#include "graph.hpp"

namespace Ats {

    class Control{
    private:
	RefPtr<IOChannel> in;
	RefPtr<IOChannel> out;
	
    public:
	Control ();
	Control (const Control&) = delete;
	Control (Control&&) = delete;

	sigc::signal<void,const string&> msg_recieved;

	std::string recv_msg ();
	void send_msg (const std::string&);
	
	void connect (const Options&);
	void connect (const Graph&);
    };

};

#endif /* CONTROL_H */
