#ifndef CONTEXT_H
#define CONTEXT_H

#include "probe.hpp"
#include "options.hpp"
#include "graph.hpp"

#include <exception>
#include <vector>
#include <glibmm.h>
#include <gstreamermm.h>

using namespace Glib;

namespace Ats {

    class Context {
    public:
	struct Size_error : std::exception {};
	
    private:
	Graph         graph;
	vector<Probe> probes;
	Options       opts;
	
	RefPtr<MainLoop> main_loop;
	

    public:
	Context(uint size);
	Context() : Context(1) {}

	void run() { main_loop->run(); }
    };

};

#endif /* CONTEXT_H */
