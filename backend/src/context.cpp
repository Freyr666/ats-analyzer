#include "context.hpp"
#include "json.hpp"

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

    control.received_json.connect(sigc::mem_fun(this, &Context::of_json));
    control.received_msgpack.connect(sigc::mem_fun(this, &Context::of_msgpack));
    
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

string
Context::to_msgpack() const {
    return "todo";
}

void
Context::of_json(const string& j) {
    using json = nlohmann::json;
    auto js = json::parse(j);
    // TODO throw Wrong_json
    if (! js.is_object()) return;

    for (json::iterator el = js.begin(); el != js.end(); ++el) {
	if (el.key() == "options" && el.value().is_object()) {
	    opts.of_json(el.value().dump());
	} else if (el.key() == "graph" && el.value().is_object()) {
	    graph.of_json(el.value().dump());
	} 
    }
    
    talk.emit(*this);
}

void
Context::of_msgpack(const string&) {
    // TODO
    talk.emit(*this);
}
