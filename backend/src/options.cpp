#include "options.hpp"

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
    updated.emit(*this);
}

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

void
of_json(const string&) {
    
}
