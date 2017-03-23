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

string
Options::to_msgpack() const {
    return "todo";
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
        if (el.key() == "option" && el.value().is_string()) {
            o_set = true; // not so serious option
        } else if (el.key() == "other option") {
            o_destr_set = true; // serious option
        } 
    }

    if (o_set)
        set.emit(*this);
    if (o_destr_set)
        destructive_set(*this);
}

void
Options::of_msgpack(const string&) {

    destructive_set(*this);
    set.emit(*this);
}
