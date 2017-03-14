#ifndef METADATA_H
#define METADATA_H

#include <string>
#include <vector>
#include <gstreamermm.h>
#include <glibmm.h>

using namespace std;

namespace Ats {

    struct Meta_pid {	
	uint    pid;
	uint    type;
	string  codec;
	// Options
	bool   to_be_analyzed;
	
	Meta_pid (uint p, uint t, string c) : pid(p), type(t), codec(c), to_be_analyzed(true) {}
	~Meta_pid () {}

	string to_string () const;
    };

    struct Meta_channel {
	uint             number;
	string           service_name;
        string           provider_name;
	vector<Meta_pid> pids;

	Meta_channel (uint n, string s, string p) : number(n), service_name(s), provider_name(p) {}
	Meta_channel (uint n) : number(n) {}
	~Meta_channel () {}

	Meta_pid*     find_pid (uint pid);

	bool   to_be_analyzed () const;
	void   append_pid (Meta_pid&& p) { pids.push_back(p); }
	uint   pids_num () { return pids.size(); }
	string to_string () const;
    };
    
    struct Metadata {
	uint                 stream;
	vector<Meta_channel> channels;

	Metadata (uint s) : stream(s) {}
	Metadata () : Metadata(0) {}
	//Metadata (Metadata&& m) : stream(m.stream),channels(std::move(m.channels)) {}
	//Metadata (const Metadata& m) : stream(m.stream),channels(m.channels) {}
	//~Metadata () {}

	Metadata& operator=(const Metadata&) = default;

	Meta_pid*     find_pid (uint chan, uint pid);
	const Meta_pid* find_pid (uint chan, uint pid) const;
	Meta_pid*     find_pid (uint pid);
	const Meta_pid* find_pid (uint pid) const;
	Meta_channel* find_channel (uint chan);
	const Meta_channel* find_channel (uint chan) const;

	void   clear () { channels.clear(); }
	void   append_channel (Meta_channel&& c) { channels.push_back(c); }
	uint   channels_num () const { return channels.size(); }
	bool   is_empty() const { return channels.empty(); }
	bool   to_be_analyzed () const;
	string to_string () const;

	void   for_analyzable (std::function<void(const Meta_channel&)>) const;
    };
};

#endif /* METADATA_H */
