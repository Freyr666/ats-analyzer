#ifndef GRAPH_H
#define GRAPH_H

#include <string>
#include <gstreamermm.h>
#include <glibmm.h>
#include <functional>

#include "chatterer.hpp"
#include "metadata.hpp"
#include "options.hpp"
#include "settings.hpp"

using namespace std;
using namespace Glib;

namespace Ats {
    
    class Graph : public Chatterer {
	
    public:
	Graph() {}
	Graph(const Graph&) = delete;
	Graph(Graph&&) = delete;
	virtual ~Graph() {}
	
	void       set(const Options&);
	void       reset();
	void       apply_options(const Options&);
	void       apply_settings(const Settings&);
	void       set_state(Gst::State);
	Gst::State get_state();

	void   connect(Options& o);
	void   connect(Settings& o);
	
	// Chatterer
	string to_string() const;
	string to_json() const;
	void   of_json(const string&);
	string to_msgpack() const;
	void   of_msgpack(const string&);
	
    private:
	RefPtr<Gst::Pipeline> pipe;
	RefPtr<Gst::Bus>      bus;
	string                address = "udp://224.1.2.3:1234";
	
	static RefPtr<Gst::Bin> create_root(const Metadata&);
	static RefPtr<Gst::Bin> create_branch(const uint,
					      const uint,
					      const Metadata&);
    };

};

#endif /* GRAPH_H */
