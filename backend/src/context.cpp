#include "context.hpp"
#include "schema.hpp"

using namespace Glib;
using namespace Ats;

Context::Context(Initial init) : graph("graph"), options("options"), settings("settings"),
                                 j_schema(compose_schema()) {
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
    connect (graph.get_wm());

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
        json j = json{{c.name,c.serialize()}};
        if (msg_type == Msg_type::Msgpack) {
            std::vector<uint8_t> msgpack = json::to_msgpack(j);
            rval = std::string(msgpack.begin(), msgpack.end());
        }
        else rval = j.dump();
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
    case Msg_type::Msgpack:
        json j = json{{"error", s}};
        if (msg_type == Msg_type::Msgpack) {
            std::vector<uint8_t> msgpack = json::to_msgpack(j);
            rval = std::string(msgpack.begin(), msgpack.end());
        }
        else rval = j.dump();
        break;
    }
    send_err.emit(rval);
}

void
Context::dispatch(const std::string& s) {
    using Df = Chatterer::Deserializer_failure;

    json j;
    json rval;

    if (msg_type == Msg_type::Msgpack) {
        try {
            std::vector<uint8_t> msgpack(s.begin(), s.end());
            j = json::from_msgpack(msgpack);
        } catch (const std::exception& e) {
            throw Df (std::string("Top-level MsgPack is corrupted: ") + e.what());
        }
    }
    else { /* Debug or Json */
        try {
            j = json::parse(s);
        } catch (const std::exception& e) {
            throw Df (std::string("Top-level JSON is corrupted: ") + e.what());
        }
    }

    /* Validate incoming json.
       This will throw an exception in case if json is bad */
    try {
        validate(j, j_schema);
    } catch (Validator_failure& e) {
        log(e.what());
        return;
    }

    for (json::iterator it = j.begin(); it != j.end(); ++it) {
        auto chatterer = get_chatterer(it.key());
        if (chatterer) chatterer->deserialize(it.value());
        // get request
        else if ((it.key() == "get") && it.value().is_string()) {
            auto chatterer = get_chatterer(it.value().get<std::string>());
            if (chatterer) {
                rval[chatterer->name] = chatterer->serialize();
            }
        }
    }
    // send reply if rval is not emply
    if (! rval.empty()) send.emit(rval);
}

