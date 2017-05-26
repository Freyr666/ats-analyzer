#include "options.hpp"
#include "json.hpp"
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

    // FIXME remove
    json j = serialize();
    rval = j.dump();

    return rval;
}

json
Options::serialize() const {
    json j = json{{"prog_list", data},
                  {"resolution", {{"width", resolution.first},
                                  {"height", resolution.second}}},
                  {"bg_color", background_color}};
    return j;
}

void
Options::deserialize(const json& j) {
    bool o_set = false;
    bool o_destr_set = false;

    if (o_destr_set) destructive_set(*this);
    else if (o_set) set.emit(*this);
}

string
Options::to_json() const {
    return "";
}

void
Options::of_json(json& js) {
}

string
Options::to_msgpack() const {
    return "todo";
}

void
Options::of_msgpack(const string&) {
}

