#include "context.hpp"
#include "json.hpp"

using namespace Glib;
using namespace Ats;

Context::Context(Initial init) {
    uint size = init.uris.size();
    
    if (size < 1) throw Context::Size_error();

    main_loop = Glib::MainLoop::create();
    
    probes.reserve(size);
    
    for (uint i = 0; i < size; i++) {
        probes.push_back(unique_ptr<Probe>(new Probe(i,init.uris[i])));
        options.connect(*probes[i]);
        probes[i]->set_state(Gst::STATE_PLAYING);
    }

    settings.init(init);
    graph.apply_settings(settings);
    if (init.msg_type) control.set_msg_type(*init.msg_type);

    control.connect (settings);
    control.connect (options);
    control.connect (graph);
    control.connect (*this);

    options.updated.connect(sigc::mem_fun(this, &Context::talk));
    
    control.received_json.connect(sigc::mem_fun(this, &Context::of_json));
    control.received_msgpack.connect(sigc::mem_fun(this, &Context::of_msgpack));
    
    graph.connect(options);
    graph.connect(settings);

    /*
      Glib::signal_timeout().connect([this](){
      graph.apply(options);
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

string
Context::to_string() const {
    string rval = graph.to_string();
    rval += options.to_string();
    rval += settings.to_string();
    return rval;
}

string
Context::to_json() const {
    string rval = "{\"graph\":";
    rval += graph.to_json();
    rval += ",\"options\":";
    rval += options.to_json();
    rval += ",\"settings\":";
    rval += settings.to_json();
    rval += "}";
    return rval;
}

string
Context::to_msgpack() const {
    return "todo";
}

void
Context::of_json(const string& j) {
    using json = nlohmann::json;
    auto js = json::parse(j);
    // TODO throw Wrong_json maybe?
    if (! js.is_object()) return;

    for (json::iterator el = js.begin(); el != js.end(); ++el) {
	if (el.key() == "options" && el.value().is_object()) {
	    options.of_json(el.value().dump());
	} else if (el.key() == "settings" && el.value().is_object()) {
	    settings.of_json(el.value().dump());
	} else if (el.key() == "graph" && el.value().is_object()) {
	    graph.of_json(el.value().dump());
	} 
    }
    
    talk();
}

void
Context::of_msgpack(const string&) {
    // TODO
}
