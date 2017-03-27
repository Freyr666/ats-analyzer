#include "options.hpp"
#include "settings.hpp"
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
    else throw Serializer_failure ();
}

void
Options::of_json(const string& j) {
    bool o_set = false;
    bool o_destr_set = false;
    
    using json = nlohmann::json;
    auto js = json::parse(j);

    // TODO throw Wrong_json
    if (! js.is_object()) return;

    for (json::iterator el = js.begin(); el != js.end(); ++el) {
        const std::string k = el.key();
        auto v = el.value();
        if (k == "prog_list") {
            if (!v.is_array()) throw;
            for (json::iterator s_it = v.begin(); s_it != v.end(); ++s_it) {
                auto j_stream = s_it.value();
                if (!j_stream.is_object()) throw;
                if (!j_stream["stream"].is_number()) throw;
                if (!j_stream["channels"].is_array()) throw;
                
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
                        if (!j_channel.is_object()) throw;
                        if (!j_channel["number"].is_number()) throw;
                        if (!j_channel["pids"].is_array() ||
                            !j_channel["pids"].is_null()) throw;

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
                                if (!j_pid["pid"].is_number()) throw;
                                if (!j_pid.is_object()) throw;

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
                                            if (!v.is_boolean()) throw;
                                            matching_pid->to_be_analyzed = v.get<bool>();
                                            o_destr_set = true;
                                        }
                                        if (k == "type") {
                                            if (!v.is_string()) throw;
                                            string type_str = v.get<std::string>();
                                            Meta_pid::Type type =
                                                (type_str == "video") ? Meta_pid::Type::Video :
                                                (type_str == "audio") ? Meta_pid::Type::Audio :
                                                (type_str == "subtitles") ? Meta_pid::Type::Subtitles :
                                                (type_str == "teletext") ? Meta_pid::Type::Teletext :
                                                (type_str == "empty") ? Meta_pid::Type::Empty :
                                                throw;
                                            if (type != matching_pid->type) throw;
                                        }
                                        if (k == "stream_type") {
                                            if (!v.is_number()) throw;
                                            if (matching_pid->stream_type != v.get<int>())
                                                throw;
                                        }
                                        if (k == "stream_type_name") {
                                            if (!v.is_string()) throw;
                                            if (matching_pid->stream_type_name != v.get<std::string>())
                                                throw;
                                        }
                                        if (k == "position") {
                                            if (!v.is_object()) throw;

                                            for (json::iterator pos_it = v.begin();
                                                 pos_it != v.end();
                                                 ++pos_it) {

                                                const string pos_k = pos_it.key();
                                                auto pos_v = pos_it.value();

                                                if (pos_k == "x") {
                                                    if (!pos_v.is_number()) throw;
                                                    matching_pid->position.x = pos_v.get<int>();
                                                    o_set = true;
                                                }
                                                else if (pos_k == "y") {
                                                    if (!pos_v.is_number()) throw;
                                                    matching_pid->position.y = pos_v.get<int>();
                                                    o_set = true;
                                                }
                                                else if (pos_k == "width") {
                                                    if (!pos_v.is_number()) throw;
                                                    matching_pid->position.width = pos_v.get<int>();
                                                    o_set = true;
                                                }
                                                else if (pos_k == "height") {
                                                    if (!pos_v.is_number()) throw;
                                                    matching_pid->position.height = pos_v.get<int>();
                                                    o_set = true;
                                                }
                                            }
                                        }
                                    }
                                }
                                else throw;
                            }
                        }
                        else throw;
                     }
                 }
                 else throw;
             }
        }
        else if (k == "resolution") {
            if (!v.is_object()) return;
            for (json::iterator it = v.begin(); it != v.end(); ++it) {
                const std::string rk = it.key();
                auto rv = it.value();
                if (rk == "width") {
                    if (!rv.is_number()) return;
                    resolution.first = rv.get<int>();
                    o_destr_set = true;
                }
                else if (rk == "height") {
                    if (!rv.is_number()) return;
                    resolution.second = rv.get<int>();
                    o_destr_set = true;
                }
            }
        }
        else if (k == "background_color") {
            if (!v.is_number()) return;
            background_color = v.get<int>();
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
