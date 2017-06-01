#include "context.hpp"
#include "json.hpp"

using namespace Glib;
using namespace Ats;

using json = nlohmann::json;

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
    std::string rval = "";


    switch (msg_type) {
    case Msg_type::Debug:
        rval = c.name + " : ";
        rval += c.to_string();
        break;
    case Msg_type::Json:
    case Msg_type::Msgpack:
        rval = "{" + c.name;
        rval += ":";
        rval += c.serialize();
        rval += "}";

        if (msg_type == Msg_type::Msgpack) {
            json j = json::parse(rval);
            std::vector<uint8_t> msgpack = json::to_msgpack(j);
            rval = std::string(msgpack.begin(), msgpack.end());
        }
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
    using Df = Chatterer::Deserializer_failure;

    json j;
    json j_schema;
    
    switch (msg_type) {
    case Msg_type::Debug:
        break;
    case Msg_type::Json:
    case Msg_type::Msgpack:
        if (msg_type == Msg_type::Msgpack) {
            try {
                std::vector<uint8_t> msgpack(s.begin(), s.end());
                j = json::from_msgpack(msgpack);
            } catch (const std::exception& e) {
                throw Df (std::string("Top-level Msgpack is corrupted: ") + e.what());
            }
        }
        else {
            try {
                j = json::parse(s);
            } catch (const std::exception& e) {
                throw Df (std::string("Top-level JSON is corrupted: ") + e.what());
            }
        }

        /* get json schema */
        j_schema = json::parse(JSON_SCHEMA);
        /* Validate incoming json.
           This will throw an exception in case if json is bad */
        Chatterer::validate(j, j_schema);

        if(j.find(options.name) != j.end()) options.deserialize(j.at(options.name));
        if(j.find(settings.name) != j.end()) settings.deserialize(j.at(settings.name));
        if(j.find(graph.name) != j.end()) graph.deserialize(j.at(graph.name));
        break;
    }
}

