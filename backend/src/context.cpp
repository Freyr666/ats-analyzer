#include "context.hpp"

using namespace Glib;
using namespace Ats;

Context::Context(uint size) {
    if (size < 1) throw Context::Size_error();

    main_loop = Glib::MainLoop::create();
    probes.reserve(size);
    for (uint i = 0; i < size; i++) {
	probes.push_back(Probe(i));
	opts.connect(probes[i]);
	probes[i].set_state(Gst::STATE_PLAYING);
    }

    graph.connect(opts);
    /*
    Glib::signal_timeout().connect([&g, &opts](){
	    g.apply(opts);
	    return false;
	},
	2000);

    Glib::signal_timeout().connect([&g, &opts](){
	    g.reset();
	    return false;
	},
	10000);

    Glib::signal_timeout().connect([&g, &opts](){
	    g.apply(opts);
	    return false;
	},
	20000);
    */
}
