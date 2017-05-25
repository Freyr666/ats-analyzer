#include "context.hpp"
#include "json.hpp"

using namespace Glib;
using namespace Ats;

Context::Context(Initial init) : graph("graph"), options("options"), settings("settings") {
    uint size = init.uris.size();

    if (init.msg_type) msg_type  = *init.msg_type;
    
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

    control.connect (settings);
    control.connect (options);
    control.connect (graph);
    control.connect ((Logger&) *this);

    control.connect ((Chatterer_proxy&) *this);

    connect (settings);
    connect (options);
    connect (graph);

    control.received.connect(sigc::mem_fun(this,&Chatterer_proxy::dispatch));
    
    options.connect(graph);
    
    graph.connect(options);
    graph.connect(settings);
    
}

void
Context::forward_talk(const Chatterer& c) {
    std::string rval;
    
    switch (msg_type) {
    case Msg_type::Debug:
	rval = c.name + " : ";
	rval += c.to_string();
	break;
    case Msg_type::Json:
	rval = "{" + c.name;
	rval += ":";
        rval += c.to_json();
	rval += "}";
	break;
    case Msg_type::Msgpack:
        rval = c.name + c.to_msgpack();
	break;
    }
    send.emit(rval);
}
void
Context::forward_error(const std::string& s) {
    std::string rval;

    switch (msg_type) {
    case Msg_type::Debug:
	rval = "Error: " + s;
	break;
    case Msg_type::Json: 
        rval = "{error: " + s;
	rval += "}";
	break;
    case Msg_type::Msgpack:
        rval = s;
	break;
    }
    send_err.emit(rval);
}
void
Context::dispatch(const std::string& s) {
    using json = nlohmann::json;
    using Df = Chatterer::Deserializer_failure;
    json js;
    
    switch (msg_type) {
    case Msg_type::Debug:
    case Msg_type::Json:
	js = json::parse(s);
	if (! js.is_object())
	    throw Df (std::string("Top-level JSON") + Df::expn_object);
	for (json::iterator el = js.begin(); el != js.end(); ++el) {
	    if (el.key() == "options" && el.value().is_object()) {
		options.of_json(el.value());
	    } else if (el.key() == "settings" && el.value().is_object()) {
		settings.of_json(el.value());
	    } else if (el.key() == "graph" && el.value().is_object()) {
		graph.of_json(el.value());
	    }
	}
	break;
    case Msg_type::Msgpack:
	break;
    }
}

