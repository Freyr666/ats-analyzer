#ifndef CONTROL_H
#define CONTROL_H

#include <gstreamermm.h>

#include "options.hpp"
#include "graph.hpp"

namespace Ats {

    class Control{

    public:

	Control ();
	Control (const Control&) = delete;
	Control (Control&&) = delete;
	void run ();

	sigc::signal<void,Options&> msg_recieved;
	void send_msg (const std::string&);
	
	void connect (const Options&);
	void connect (const Graph&);
    };

};

#endif /* CONTROL_H */
