#include "options.hpp"
#include "graph.hpp"
#include "probe.hpp"

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
            return m.to_be_analyzed();
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
    updated.emit();
    send.emit(*this);
}

void
Options::set_pid(const uint stream,
                 const uint chan,
                 const uint pid,
                 Meta_pid::Pid_type v) {
    auto s = find_stream(stream);
    auto p = s->find_pid(chan, pid);
    p->set(v);
    updated.emit(); // TODO add validation
    send.emit(*this);
}

Metadata*
Options::find_stream (uint stream) {
    for (Metadata& m : data) {
        if (m.stream == stream) return &m;
    }
    return nullptr;
}

const Metadata*
Options::find_stream (uint stream) const {
    for (const Metadata& m : data) {
        if (m.stream == stream) return &m;
    }
    return nullptr;
}

// Connections

void
Options::connect(Probe& p) { p.updated.connect(
	sigc::mem_fun(this, &Options::set_data));
}

void
Options::connect(Graph& g) { g.set_pid.connect(
	sigc::mem_fun(this, &Options::set_pid));
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
    string rval = "Streams:\n\t\t";
    rval += streams;
    rval += "\tResolution:\n\t\t";
    rval += std::to_string(resolution.first);
    rval += "x";
    rval += std::to_string(resolution.second);
    rval += "\n";
    rval += "\tBackground color:\n\t\t";
    rval += std::to_string(bg_color);
    rval += "\n\n";
    return rval;
}

json
Options::serialize() const {
    json j = json{{"prog_list", data},
                  {"resolution", {{"width", resolution.first},
                                  {"height", resolution.second}}},
                  {"bg_color", bg_color}};
    return j;
}

void
Options::deserialize(const json& j) {
    constexpr const char* metadata_key = "prog_list";
    constexpr const char* resolution_key = "resolution";
    constexpr const char* bg_color_key = "bg_color";

    bool o_set = false;
    bool o_destr_set = false;

    /* if metadata present in json */
    if (j.find(metadata_key) != j.end()) {
        for (json::const_iterator it = j.cbegin(); it != j.cend(); ++it) {
            auto j_stream = it.value();
            uint stream_id = j_stream.at("stream").get<uint>();
            auto matching_stream = find_stream(stream_id);

            if (matching_stream == nullptr) {
                // TODO maybe add log message here
                continue;
            }

            auto j_channels = j_stream.at("channels");
            for (json::iterator c_it = j_channels.begin(); c_it != j_channels.end(); ++c_it) {
                auto j_channel = c_it.value();
                uint channel_id = j_channel.at("number").get<uint>();
                auto matching_channel = matching_stream->find_channel(channel_id);

                if(matching_channel == nullptr) {
                    // TODO maybe add log message here
                    continue;
                }

                auto j_pids = j_channel.at("pids");
                for (json::iterator p_it = j_pids.begin(); p_it != j_pids.end(); ++p_it) {
                    auto j_pid = p_it.value();
                    uint pid = j_pid.at("pid").get<uint>();
                    auto matching_pid = matching_stream->find_pid(channel_id, pid);

                    if(matching_pid == nullptr) {
                        // TODO maybe add log message here
                        continue;
                    }

                    matching_pid->to_be_analyzed = j_pid.at("to_be_analyzed").get<bool>();
                    matching_pid->position = j_pid.at("position").get<Ats::Position>();
                    o_destr_set = true;
                }
            }
        }
    } // TODO maybe add log message at else clause

    /* if multiscreen resolution present in json */
    if (j.find(resolution_key) != j.end()) {
        resolution.first = j.at(resolution_key).at("width").get<uint>();
        resolution.second = j.at(resolution_key).at("height").get<uint>();
        o_set = true;
    } // TODO maybe add log message at else clause

    /* if multiscreen background color present in json */
    if (j.find(bg_color_key) != j.end()) {
        set_value_from_json(j,*this,bg_color,uint);
        o_set = true;
    } // TODO maybe add log message at else clause

    if (o_destr_set) destructive_set(*this);
    else if (o_set) set.emit(*this);
}
