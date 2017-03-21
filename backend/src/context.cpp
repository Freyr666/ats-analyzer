#include "context.hpp"

using namespace Glib;
using namespace Ats;

Context::Context(uint size) {
    if (size < 1) throw Context::Size_error();

    main_loop = Glib::MainLoop::create();
    
    probes.reserve(size);
    
    for (uint i = 0; i < size; i++) {
        probes.push_back(unique_ptr<Probe>(new Probe(i)));
        opts.connect(*probes[i]);
        probes[i]->set_state(Gst::STATE_PLAYING);
    }

    control.connect (opts);
    control.connect (graph);
    control.connect (*this);
    // graph.connect(opts);
    
    Glib::signal_timeout().connect([this](){
            graph.apply(opts);
            return false;
        },
        2000);
    /*
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

string
Context::to_string() const {
    return "todo";
}

string
Context::to_json() const {
    return "todo";
}

void
Context::of_json(const string&) {
    talk.emit(*this);
}

string
Context::to_msgpack() const {
    return "todo";
}

void
Context::of_msgpack(const string&) {
    talk.emit(*this);
}
