#ifndef GRAPH_H
#define GRAPH_H

#include "options.hpp"

#include <string>
#include <gstreamermm.h>
#include <glibmm.h>

using namespace std;
using namespace Glib;

namespace Ats {

    class Graph {
    
    public:
	Graph() {}
	~Graph() {}

	void   connect(Options& o) { o.updated.connect(
		sigc::mem_fun(this, &Graph::apply));
	}
	
	void   apply(const Options&);
	void   set_state(Gst::State);
	string to_string() const;
	
    private:
	RefPtr<Gst::Pipeline> pipe;
	RefPtr<Gst::Bus>      bus;
	
    };
    
};

#endif /* GRAPH_H */
