#include "options.hpp"
#include "json.hpp"

#include <cstdio>
#include <string>
#include <algorithm>
#include <iostream>

using namespace std;
using namespace Ats;

/* ---------- Options -------------------- */

bool
Options::is_empty () const {
    if (data.empty()) return true;

    auto v = find_if (data.begin(), data.end(), [](const Metadata& m){
            return ! m.to_be_analyzed();
        });
    return v == data.end();
}

void
Options::set_data(const Metadata& m) {
    if (data.empty()) {
        data.push_back(Metadata(m));
    } else {
        auto v = find_if(data.begin(),data.end(),[&m](Metadata& el) {
                return el.stream == m.stream;
            });
        
        if ( (v != data.end()) && (v->stream == m.stream) ) {
            *v = m;
        } else {
            data.push_back(Metadata(m));
        }
    }
    talk();
}

// Chatter implementation

string
Options::to_string() const {
    string streams = "";
    for_each(data.begin(),data.end(),[&streams](const Metadata& m){
            streams += m.to_string();
            streams += "\n";
        });
    Ats::add_indent(streams);
    string rval = "Options:\n\tStreams:\n\t\t";
    rval += streams;
    rval += "\tResolution:\n\t\t";
    rval += std::to_string(resolution.first);
    rval += "x";
    rval += std::to_string(resolution.second);
    rval += "\n";
    rval += "\tBackground color:\n\t\t";
    rval += std::to_string(background_color);
    rval += "\n\n";
    return rval;
}

string
Options::to_json() const {
    using Sf = Chatterer::Serializer_failure;
    constexpr int size = 1024 * 8;

    char buffer[size];
    constexpr const char* fmt = "{"
        "\"prog_list\":[%s],"
        "\"resolution\":{\"width\":%d,\"height\":%d},"
        "\"background_color\":%d"
        "}";

    string s = "";
    for (auto it = data.begin(); it != data.end(); ++it) {
        if ( it != data.begin() )
            s += ",";
        s += it->to_json();
    }
    int n = snprintf (buffer, size, fmt, s.c_str(),
                      resolution.first, resolution.second,
                      background_color);
    if ( (n>=0) && (n<size) ) return string(buffer);
    else throw Sf (string("Options: ") + Sf::expn_overflow + std::to_string(size));
}

void
Options::of_json(const string& j) {
    using Df = Chatterer::Deserializer_failure;
    using json = nlohmann::json;

    constexpr const char* div = "::";
    bool o_set = false;
    bool o_destr_set = false;
    
    auto js = json::parse(j);

    if (! js.is_object()) throw Df(string("Options") + Df::expn_object);

    for (json::iterator el = js.begin(); el != js.end(); ++el) {
        const std::string jk = el.key();
        auto jv = el.value();
        
        if (jk == "prog_list") {
            if (!jv.is_array()) throw Df(jk + Df::expn_object);

            for (json::iterator s_it = jv.begin(); s_it != jv.end(); ++s_it) {
                auto j_stream = s_it.value();
                if (!j_stream.is_object())
                    throw Df(jk + div + "Metadata" + Df::expn_object);
                if (!j_stream["stream"].is_number())
                    throw Df(jk + div + "Metadata" + div + "stream" + Df::expn_object);
                if (!j_stream["channels"].is_array())
                    throw Df(jk + div + "Metadata" + div + "channels" + Df::expn_object);

                int stream_id = j_stream["stream"].get<int>();
                auto matching_stream = find_if(data.begin(),
                                               data.end(),
                                               [&stream_id](const Metadata& m) {
                                                   return m.stream == stream_id;
                                               });

                // modify existing stream
                if (matching_stream != data.end()) {
                    auto j_channels = j_stream["channels"];
                    for (json::iterator c_it = j_channels.begin(); c_it != j_channels.end(); ++c_it) {
                        auto j_channel = c_it.value();
                        if (!j_channel.is_object())
                            throw Df(jk + div + "Meta_channel" + Df::expn_object);
                        if (!j_channel["number"].is_number())
                            throw Df(jk + div + "Meta_channel" + div + "number" + Df::expn_object);
                        if (!j_channel["pids"].is_array() ||
                            !j_channel["pids"].is_null())
                            throw Df(jk + div + "Meta_channel" + div + "number" + \
                                     Df::expn_object + " or" + Df::expn_null);

                        int number = j_channel["number"].get<int>();
                        auto matching_channel = find_if(matching_stream->channels.begin(),
                                                        matching_stream->channels.end(),
                                                        [&number](const Meta_channel& c) {
                                                            return c.number == number;
                                                        });

                        if (matching_channel != matching_stream->channels.end()) {
                            if (j_channel["service_name"].is_string())
                                matching_channel->service_name = j_channel["service_name"].get<std::string>();
                            if (j_channel["provider_name"].is_string())
                                matching_channel->provider_name = j_channel["provider_name"].get<std::string>();
                            auto j_pids = j_channel["pids"];
                            for (json::iterator p_it = j_pids.begin(); p_it != j_pids.end(); ++p_it) {
                                auto j_pid = p_it.value();
                                if (!j_pid.is_object())
                                    throw Df(jk + div + "Meta_pid" + Df::expn_object);
                                if (!j_pid["pid"].is_number())
                                    throw Df(jk + div + "Meta_pid" + div + "pid" + Df::expn_number);

                                int pid = j_pid["pid"].get<int>();
                                auto matching_pid = find_if(matching_channel->pids.begin(),
                                                            matching_channel->pids.end(),
                                                            [&pid](const Meta_pid& p) {
                                                                return p.pid == pid;
                                                            });
                                if (matching_pid != matching_channel->pids.end()) {

                                    for (json::iterator it = j_pid.begin(); it != j_pid.end(); ++it) {
                                        const string k = it.key();
                                        auto v = it.value();

                                        if (k == "to_be_analyzed") {
                                            if (!v.is_boolean())
                                                throw Df(jk + div + "Meta_pid" + div + k + \
                                                         Df::expn_bool);
                                            matching_pid->to_be_analyzed = v.get<bool>();
                                            o_destr_set = true;
                                        }
                                        if (k == "type") {
                                            if (!v.is_string())
                                                throw Df(jk + div + "Meta_pid" + div + k + \
                                                         Df::expn_string);
                                            string type_str = v.get<std::string>();
                                            Meta_pid::Type type =
                                                (type_str == "video") ? Meta_pid::Type::Video :
                                                (type_str == "audio") ? Meta_pid::Type::Audio :
                                                (type_str == "subtitles") ? Meta_pid::Type::Subtitles :
                                                (type_str == "teletext") ? Meta_pid::Type::Teletext :
                                                (type_str == "empty") ? Meta_pid::Type::Empty :
                                                throw Df(type_str + " is not correct Meta_pid type");
                                            if (type != matching_pid->type)
                                                throw Df(string("Meta_pid types do not match"));
                                        }
                                        if (k == "stream_type") {
                                            if (!v.is_number())
                                                throw Df(jk + div + "Meta_pid" + div + k + \
                                                         Df::expn_number);
                                            if (matching_pid->stream_type != v.get<int>())
                                                throw Df(string("Meta_pid stream types do not match"));
                                        }
                                        if (k == "stream_type_name") {
                                            if (!v.is_string())
                                                throw Df(jk + div + "Meta_pid" + div + k + \
                                                         Df::expn_string);
                                            if (matching_pid->stream_type_name != v.get<std::string>())
                                                throw Df(string("Meta_pid stream type names do not match"));
                                        }
                                        if (k == "position") {
                                            if (!v.is_object())
                                                throw Df(jk + div + "Meta_pid" + div + k + \
                                                         Df::expn_object);

                                            for (json::iterator pos_it = v.begin();
                                                 pos_it != v.end();
                                                 ++pos_it) {

                                                const string pos_k = pos_it.key();
                                                auto pos_v = pos_it.value();

                                                if (pos_k == "x") {
                                                    if (!pos_v.is_number())
                                                        throw Df(jk + div + "Meta_pid" + div + k + \
                                                                 + div + pos_k + Df::expn_number);
                                                    matching_pid->position.x = pos_v.get<int>();
                                                    o_set = true;
                                                }
                                                else if (pos_k == "y") {
                                                    if (!pos_v.is_number())
                                                        throw Df(jk + div + "Meta_pid" + div + k + \
                                                                 + div + pos_k + Df::expn_number);
                                                    matching_pid->position.y = pos_v.get<int>();
                                                    o_set = true;
                                                }
                                                else if (pos_k == "width") {
                                                    if (!pos_v.is_number())
                                                        throw Df(jk + div + "Meta_pid" + div + k + \
                                                                 + div + pos_k + Df::expn_number);
                                                    matching_pid->position.width = pos_v.get<int>();
                                                    o_set = true;
                                                }
                                                else if (pos_k == "height") {
                                                    if (!pos_v.is_number())
                                                        throw Df(jk + div + "Meta_pid" + div + k + \
                                                                 + div + pos_k + Df::expn_number);
                                                    matching_pid->position.height = pos_v.get<int>();
                                                    o_set = true;
                                                }
                                            }
                                        }
                                    }
                                }
                                else throw Df(string("PID ") + std::to_string(pid) + \
                                              " does not exist in stream " + \
                                              std::to_string(stream_id) + \
                                              ", channel " + std::to_string(number));
                            }
                        }
                        else throw Df(string("Channel ") + std::to_string(number) + \
                                      " does not exist in stream " + \
                                      std::to_string(stream_id));
                    }
                }
                else throw Df(string("Stream ") + std::to_string(stream_id) + " does not exist");
            }
        }
        else if (jk == "resolution") {
            if (!jv.is_object()) throw Df(jk + Df::expn_object);
            for (json::iterator it = jv.begin(); it != jv.end(); ++it) {
                const std::string k = it.key();
                auto v = it.value();

                if (k == "width") {
                    if (!v.is_number()) throw Df(jk + div + k + Df::expn_number);
                    resolution.first = v.get<int>();
                    o_destr_set = true;
                }
                else if (k == "height") {
                    if (!v.is_number()) throw Df(jk + div + k + Df::expn_number);
                    resolution.second = v.get<int>();
                    o_destr_set = true;
                }
            }
        }
        else if (jk == "background_color") {
            if (!jv.is_number()) throw Df(jk + Df::expn_number);
            background_color = jv.get<int>();
            o_set = true;
        }
    }

    if (o_set) set.emit(*this);
    if (o_destr_set) destructive_set(*this);
}

string
Options::to_msgpack() const {
    return "todo";
}

void
Options::of_msgpack(const string&) {

    destructive_set(*this);
    set.emit(*this);
}
