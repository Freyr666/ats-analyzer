#include "metadata.hpp"

#include <algorithm>

using namespace std;
using namespace Ats;

// --------- Meta_pid -------------------

string
Meta_pid::to_string () const {
    string rval = "Pid: ";
    rval += std::to_string(pid);
    rval += " Type: ";
    rval += std::to_string(type);
    rval += " Codec: ";
    rval += codec;
    return rval;
}

// --------- Meta_channel  ---------------

Meta_pid*
Meta_channel::find_pid (uint pid) {
    for (Meta_pid& p : pids) {
	if (p.pid == pid) return &p;
    }
    return nullptr;
}

string
Meta_channel::to_string () const {
    string rval = "Channel: ";
    rval += std::to_string(number);
    rval += " Service name: ";
    rval += service_name;
    rval += " Provider name: ";
    rval += provider_name;
    rval += " Pids: ";
    for_each (pids.begin(), pids.end(), [&rval](const Meta_pid& p) {
	    rval += "(";
	    rval += p.to_string();
	    rval += ");";
	});
    return rval;
}

bool
Meta_channel::to_be_analyzed () const {
    auto result = find_if (pids.begin(), pids.end(), [](const Meta_pid& p){
	    return p.to_be_analyzed;
	});
    return result->to_be_analyzed;
}

// ---------- Metadata ---------------------


Meta_pid*
Metadata::find_pid (uint chan, uint pid) {
    for (Meta_channel& c : channels) {
	if (c.number == chan)
	    for (Meta_pid& p : c.pids) {
		if (p.pid == pid) return &p;
	    }
    }
    return nullptr;
}

Meta_channel*
Metadata::find_channel (uint chan) {
    for (Meta_channel& c : channels) {
	if (c.number == chan) return &c;
    }
    return nullptr;
}

string
Metadata::to_string () const {
    string rval = "Stream: ";
    rval += std::to_string(stream);
    rval += " Channels: ";
    for_each (channels.begin(), channels.end(), [&rval](const Meta_channel& c) {
	    rval += "[";
	    rval += c.to_string();
	    rval += "];";
	});
    return rval;
}

bool
Metadata::to_be_analyzed () const {
    auto rval = find_if (channels.begin(),channels.end(),[](const Meta_channel& c){
	    return c.to_be_analyzed();
	});
    return rval->to_be_analyzed();
}

void
Metadata::for_analyzable (std::function<void(const Meta_channel&)> fun) const {
    for_each(channels.begin(),channels.end(),[&fun](const Meta_channel& c){
	    if (c.to_be_analyzed()) fun(c);
	});
}
