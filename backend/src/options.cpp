#include "options.hpp"

#include <algorithm>
#include <iostream>

using namespace std;
using namespace Ats;

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
    cout << this->to_string() << endl;;
}

string
Options::to_string() {
    string rval = "\tOptions:\n\tStreams:\n";
    for_each(data.begin(),data.end(),[&rval](Metadata& m){
	    rval += "\n";
	    rval += m.to_string();
	    rval += "\n";
	});
    rval += "\tOther options:\n";
    rval += "Dummy: ";
    rval += std::to_string(dummy);
    rval += "\n";
    return rval;
}

string
Options::to_json() {
    return "todo";
}

void
of_json(const string&) {
    
}
