#ifndef METADATA_H
#define METADATA_H

#include <string>
#include <vector>
#include <exception>
#include <gstreamermm.h>
#include <glibmm.h>
#include <boost/variant.hpp>

#include "chatterer.hpp"

using namespace std;

namespace Ats {

    struct Position {
        int x = 0;
        int y = 0;
        int width = 0;
        int height = 0;

        bool operator== (const Position&);
        bool operator!= (const Position&);
        bool is_overlap (const Position&);
        string to_string () const;
        string to_json()   const;
        void   of_json(const string&);
    };

    struct Meta_pid {
        class Wrong_type : std::exception {};
        enum class Type {Video, Audio, Subtitles, Teletext, Empty};

	struct Empty_pid {};
	
        struct Video_pid {
            string codec;
            uint width = 0;
            uint height = 0;
            pair<uint, uint> aspect_ratio = {0, 0};
            string interlaced;
            float frame_rate = 0;

            string to_json()   const;
            void   of_json(const string&);
        };
        struct Audio_pid {
            string codec;
            string bitrate;
            uint sample_rate = 0;

            string to_json()   const;
            void   of_json(const string&);
        };
	using Pid_type = boost::variant<Audio_pid, Video_pid, Empty_pid>;

        uint pid;
        bool to_be_analyzed;
        Type type = Type::Empty;
        uint stream_type;
        string stream_type_name;
        Position position;

        Meta_pid (uint p, uint t, string c);

        static Type get_type (uint);
        const Audio_pid&  get_audio () const;
        const Video_pid&  get_video () const;
	void        set (Video_pid v) { data = v; }
	void        set (Audio_pid a) { data = a; }
	void        set (Pid_type p)  { data = p; }
        string      to_string () const;
        string      to_json ()   const;
        void        of_json (const string&);

    private:
        Pid_type data = Empty_pid();
    };

    struct Meta_channel {
        uint             number;
        string           service_name;
        string           provider_name;
        vector<Meta_pid> pids;

        Meta_channel (uint n, string s, string p) : number(n), service_name(s), provider_name(p) {}
        Meta_channel (uint n) : number(n) {}
        ~Meta_channel () {}

        Meta_pid*       find_pid (uint pid);
        const Meta_pid* find_pid (uint pid) const;

        bool   to_be_analyzed () const;
        void   append_pid (Meta_pid&& p) { pids.push_back(p); }
        uint   pids_num () { return pids.size(); }
        string to_string () const;
        string to_json()   const;
        void   of_json(const string&);
    };
    
    struct Metadata {
        uint                 stream;
	string               uri;
        vector<Meta_channel> channels;

        Metadata (string u, uint s) : stream(s), uri(u) {}
        Metadata () : Metadata("udp://224.1.2.2:1234", 0) {}
        //Metadata (Metadata&& m) : stream(m.stream),channels(std::move(m.channels)) {}
        //Metadata (const Metadata& m) : stream(m.stream),channels(m.channels) {}
        //~Metadata () {}

        Metadata& operator=(const Metadata&) = default;

        Meta_pid*           find_pid (uint chan, uint pid);
        const Meta_pid*     find_pid (uint chan, uint pid) const;
        Meta_pid*           find_pid (uint pid);
        const Meta_pid*     find_pid (uint pid) const;
        Meta_channel*       find_channel (uint chan);
        const Meta_channel* find_channel (uint chan) const;

        void   clear () { channels.clear(); }
        void   append_channel (Meta_channel&& c) { channels.push_back(c); }
        uint   channels_num () const { return channels.size(); }
        bool   is_empty() const { return channels.empty(); }
        bool   to_be_analyzed () const;
        string to_string () const;
        string to_json() const;

        void   for_analyzable (std::function<void(const Meta_channel&)>) const;
        bool   validate_grid (uint, uint) const;
    };
};

#endif /* METADATA_H */
