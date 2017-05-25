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
	rval = c.to_string(); break;
    case Msg_type::Json: 
        rval = c.to_json(); break;
    case Msg_type::Msgpack:
        rval = c.to_msgpack(); break;
    }
    send.emit(rval);
}
void
Context::forward_error(const std::string&) {
    
}
void
Context::dispatch(const std::string&) {}

/*
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
    if (! js.is_object())
        throw Df (std::string("Top-level JSON") + Df::expn_object);

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
*/
