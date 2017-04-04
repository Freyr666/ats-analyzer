#ifndef GRAPH_H
#define GRAPH_H

#include <string>
#include <gstreamermm.h>
#include <glibmm.h>
#include <functional>
#include <map>
#include <boost/variant.hpp>

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

	void   set_resolution(const pair<int,int>);
	void   set_position(int, int, int, const Position&);
	void   set_settings(const Settings&);

	void   connect(Options& o);
	void   connect(Settings& o);

	sigc::signal<void,const int,const int,const int,Meta_pid::Pid_type>   set_pid;
	
	// Chatterer
	string to_string() const;
	string to_json() const;
	void   of_json(const string&);
	string to_msgpack() const;
	void   of_msgpack(const string&);
	
    private:
	struct Node {
	    string               type;
	    RefPtr<Gst::Element> analysis;
	    RefPtr<Gst::Pad>     connected;
	};
	class Tree {
	    map<tuple<int,int,int>,Node> _tree;
	public:
	    bool  empty() {return _tree.empty();}
	    void  reset();
	    void  add(int,int, int, Node);
	    Node* get(int, int, int);
	};
	
	Tree                  elms;
	RefPtr<Gst::Element>  bg;
	RefPtr<Gst::Pad>      bg_pad;
	
	RefPtr<Gst::Pipeline> pipe;
	RefPtr<Gst::Bus>      bus;
	
	RefPtr<Gst::Bin> create_root(const Metadata&);
	RefPtr<Gst::Bin> create_branch(const int,
				       const int,
				       const int,
				       const Metadata&);

	bool             on_bus_message(const Glib::RefPtr<Gst::Bus>&,
					const Glib::RefPtr<Gst::Message>&);
    };

};

#endif /* GRAPH_H */
