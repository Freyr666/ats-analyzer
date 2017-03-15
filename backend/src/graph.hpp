#ifndef GRAPH_H
#define GRAPH_H

#include "options.hpp"

#include <string>
#include <gstreamermm.h>
#include <glibmm.h>
#include <functional>

using namespace std;
using namespace Glib;

namespace Ats {

    class Graph {
    
    public:
	Graph() {}
	Graph(const Graph&) = delete;
	Graph(Graph&&) = delete;

	void   connect(Options& o) { o.updated.connect(
		sigc::mem_fun(this, &Graph::apply));
	}
	
	void   apply(const Options&);
	void   reset();
	void   set_state(Gst::State);
	string to_string() const;
	
    private:
	RefPtr<Gst::Pipeline> pipe;
	RefPtr<Gst::Bus>      bus;
	
	static RefPtr<Gst::Bin> create_root(const Metadata&);
	static RefPtr<Gst::Bin> create_branch(const uint,
					      const uint,
					      const string,
					      const string,
					      const Metadata&);
    };
    
};

#endif /* GRAPH_H */
