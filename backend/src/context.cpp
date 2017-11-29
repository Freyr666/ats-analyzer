#include "context.hpp"
#include "schema.hpp"

using namespace Glib;
using namespace Ats;

Context::Context(Initial init) : control(init.log_level),
                                 graph("graph"),
                                 streams("streams"),
                                 settings("settings"),
                                 j_schema(compose_schema()) {
    uint size = init.uris.size();

    if (init.msg_type) msg_type  = *init.msg_type;
    
    if (size < 1) throw Context::Size_error();

    main_loop = Glib::MainLoop::create();
    
    probes.reserve(size);
    
    for (uint i = 0; i < size; i++) {
        probes.push_back(unique_ptr<Probe>(new Probe(i,init.uris[i])));
        streams.connect(*probes[i]);
        probes[i]->set_state(Gst::STATE_PLAYING);
    }

    settings.init(init);
    graph.apply_settings(settings.settings);

    control.connect (settings);
    control.connect (streams);
    control.connect (graph);
    control.connect ((Logger&) *this);

    control.connect ((Chatterer_proxy&) *this);
    
    connect (settings);
    connect (streams);
    connect (graph);
    connect (graph.get_wm());

    control.received.connect(sigc::mem_fun(this,&Chatterer_proxy::dispatch));
    
    streams.connect(graph);
    
    graph.connect(streams);
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
        } else {
            rval = j.dump();
        }
        break;
    }
    send.emit(rval);
}

std::string
Context::make_error(const std::string& s) {
    std::string rval = "";

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

    return rval;
}

void
Context::forward_error(const std::string& s) {
    std::string e = make_error(s);
    send_err.emit(e);
}

std::string
Context::dispatch(const std::vector<std::uint8_t>& data) {

    json j;
    json j_reply;
    auto f_get = [this](std::string& chatterer_name) {
        auto chatterer = get_chatterer(chatterer_name);
        if (chatterer != nullptr) {
            json j_get_reply = chatterer->serialize();
            return j_get_reply;
        } else throw Error_expn(string("object with name '") + chatterer_name + "' not found");
    };

    if (msg_type == Msg_type::Msgpack) {
        try {
            j = json::from_msgpack(data);
        } catch (const std::exception& e) {
            return make_error(std::string("top-level MsgPack is corrupted: ") + e.what());
        }
    }
    else { /* Debug or Json */
        try {
            std::string s(data.begin(), data.end());
            j = json::parse(s);
        } catch (const std::exception& e) {
            return make_error(std::string("Top-level JSON is corrupted: ") + e.what());
        }
    }

    /* Validate incoming json. This will throw an exception in case if json is bad */
    /*try {
        validate(j, j_schema);
    } catch (const std::exception& e) {
        return make_error(e.what());
        }*/

    if (j.find("get") != j.end()) {
        json j_get = j.at("get");
        j_reply = json::object();

        if(j_get.is_string()) {
            std::string chatterer_name = j_get.get<std::string>();
            try {
                j_reply[chatterer_name] = f_get(chatterer_name);
            } catch (const std::exception& e) {
                return make_error(string("Get request error: ") + e.what());
            }
        }
        else if (j_get.is_array()) {
            for (json::iterator it = j_get.begin(); it != j_get.end(); ++it) {
                std::string chatterer_name = it.value().get<std::string>();
                try {
                    j_reply[chatterer_name] = f_get(chatterer_name);
                } catch (const std::exception& e) {
                    return make_error(string("Get request error: ") + e.what());
                }
            }
        }
    }
    else if (j.find("set") != j.end()) {
        json j_set = j.at("set");
        j_reply = json::array();

        for (json::iterator it = j_set.begin(); it != j_set.end(); ++it) {
            auto chatterer = get_chatterer(it.key());
            if (chatterer != nullptr) {
                try {
                    chatterer->deserialize(it.value());
                    j_reply.push_back(it.key());
                } catch (const std::exception& e) {
                    return make_error(e.what());
                }
            }
            else return make_error(string("Set request error: object with name '") + it.key() + "' not found");
        }
    }
    else {
        return make_error("Neither 'set' nor 'get' request received");
    }

    json j_result = {{"ok",j_reply}};
    return j_result.dump();
}

