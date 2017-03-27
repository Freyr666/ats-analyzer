#ifndef CONTEXT_H
#define CONTEXT_H

#include <exception>
#include <vector>
#include <glibmm.h>
#include <gstreamermm.h>

#include "control.hpp"
#include "chatterer.hpp"
#include "probe.hpp"
#include "options.hpp"
#include "settings.hpp"
#include "graph.hpp"

using namespace std;
using namespace Glib;

namespace Ats {

    class Context : public Chatterer {
    public:
	struct Size_error : std::exception {};
	
    private:	
	Graph         graph;
	vector< unique_ptr<Probe> > probes;
	Control       control;
	Options       options;
	Settings      settings;
	
	RefPtr<MainLoop> main_loop;
  	
    public:
	Context(Initial);
	Context(const Context&) = delete;
	Context(Context&&) = delete;
	virtual ~Context() {}

	void run() { main_loop->run(); }

	// Chatterer	
	string to_string() const;	
	string to_json()   const;
	void   of_json(const string&);
	string to_msgpack()   const;
	void   of_msgpack(const string&);
    };

};

#endif /* CONTEXT_H */
