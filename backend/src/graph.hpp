#ifndef GRAPH_H
#define GRAPH_H

#include <string>
#include <gstreamermm.h>
#include <glibmm.h>
#include <functional>

#include "chatterer.hpp"
#include "metadata.hpp"
#include "options.hpp"

using namespace std;
using namespace Glib;

namespace Ats {
    
    class Graph : public Chatterer {
	
    public:
	Graph() {}
	Graph(const Graph&) = delete;
	Graph(Graph&&) = delete;
	virtual ~Graph() {}
	
	void       apply(const Options&);
	void       reset();
	void       set_state(Gst::State);
	Gst::State get_state();

	void   connect(Options& o);

	// Chatterer
	string to_string() const;
	string to_json() const;
	void   of_json(const string&);
	string to_msgpack() const;
	void   of_msgpack(const string&);
	
    private:
	RefPtr<Gst::Pipeline> pipe;
	RefPtr<Gst::Bus>      bus;
	
	static RefPtr<Gst::Bin> create_root(const Metadata&);
	static RefPtr<Gst::Bin> create_branch(const uint,
					      const uint,
					      const Metadata&);
    };

};

#endif /* GRAPH_H */
