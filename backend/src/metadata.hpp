#ifndef METADATA_H
#define METADATA_H

#include <string>
#include <vector>
#include <experimental/optional>
#include <gstreamermm.h>
#include <glibmm.h>

using namespace std;

namespace Ats {

    struct Meta_pid {
     	enum class Type {Video, Audio};
	
	uint    pid;
	Type    type;
	string  codec;

	Meta_pid (uint p, Type t, string c) : pid(p), type(t), codec(c) {}
	~Meta_pid () {}

	string to_string ();
    };

    struct Meta_channel {
	uint             number;
	string           service_name;
        string           provider_name;
	vector<Meta_pid> pids;

	Meta_channel (uint n, string s, string p) : number(n), service_name(s), provider_name(p) {}
	~Meta_channel () {}

	uint   pids_num () { return pids.size(); }
	string to_string ();
    };
    
    struct Metadata {
	uint                 stream;
	vector<Meta_channel> channels;

	Metadata (uint s) : stream(s) {}
	~Metadata ();

	experimental::optional<Meta_pid*>     find_pid (uint chan, uint pid);
	experimental::optional<Meta_channel*> find_channel (uint chan);
	uint   channels_num () { return channels.size(); }
	string to_string ();
    };
};

#endif /* METADATA_H */
