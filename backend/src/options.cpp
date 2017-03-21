#include "options.hpp"
#include "json.hpp"

#include <algorithm>
#include <iostream>

using namespace std;
using namespace Ats;

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

	if (v->stream == m.stream) {
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
    string rval = "\tOptions:\n\tStreams:\n";
    for_each(data.begin(),data.end(),[&rval](const Metadata& m){
	    rval += "\n";
	    rval += m.to_string();
	    rval += "\n";
	});
    rval += "\tOther options:\n";
    rval += "Dummy: ";
    rval += std::to_string(resolution.first);
    rval += "\n";
    return rval;
}

string
Options::to_json() const {
    return "todo";
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
