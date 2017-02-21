#include "metadata.hpp"

#include <algorithm>

using namespace std;
using namespace Ats;

// --------- Meta_pid -------------------

string
Meta_pid::to_string () {
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
Meta_channel::to_string () {
    string rval = "Channel: ";
    rval += std::to_string(number);
    rval += " Service name: ";
    rval += service_name;
    rval += " Provider name: ";
    rval += provider_name;
    rval += " Pids: ";
    for_each (pids.begin(), pids.end(), [&rval](Meta_pid& p) {
	    rval += "(";
	    rval += p.to_string();
	    rval += ");";
	});
    return rval;
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
Metadata::to_string () {
    string rval = "Stream: ";
    rval += std::to_string(stream);
    rval += " Channels: ";
    for_each (channels.begin(), channels.end(), [&rval](Meta_channel& c) {
	    rval += "[";
	    rval += c.to_string();
	    rval += "];";
	});
    return rval;
}
