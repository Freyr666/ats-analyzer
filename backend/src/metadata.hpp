#ifndef METADATA_H
#define METADATA_H

#include "json.hpp"
#include <string>
#include <vector>
#include <exception>
#include <gstreamermm.h>
#include <glibmm.h>
#include <boost/variant.hpp>

#include "chatterer.hpp"

using namespace std;

namespace Ats {

    using json = nlohmann::json;

    struct Meta_pid {
        class Wrong_type : std::exception {};
        enum class Type {Video, Audio, Empty};

        struct Empty_pid {};

        struct Video_pid {
            Video_pid () : resolution({0,0}), aspect_ratio({0,0}), frame_rate(0.)  {}
            string codec;
            pair<uint, uint> resolution;
            pair<uint, uint> aspect_ratio;
            string interlaced;
            float frame_rate;
        };
        struct Audio_pid {
            Audio_pid () : channels(0), sample_rate(0) {}
            string codec;
            string bitrate;
            uint channels;
            uint sample_rate;
        };
        using Pid_type = boost::variant<Audio_pid, Video_pid, Empty_pid>;

        uint pid;
        bool to_be_analyzed;
        Type type = Type::Empty;
        uint stream_type;
        string stream_type_name;

        Meta_pid (uint p, uint t, string c);

        static Type get_type (uint);
        const Audio_pid&  get_audio () const;
        const Video_pid&  get_video () const;
        void        set (Video_pid v) { data = v; }
        void        set (Audio_pid a) { data = a; }
        void        set (Pid_type p)  { data = p; }
        string      get_type_string() const;
        string      to_string () const;

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
        json   to_json() const;

        void   for_analyzable (std::function<void(const Meta_channel&)>) const;
    };

    // json arbitrary types conversions
    void to_json(json& j, const Meta_pid::Video_pid&);
    void to_json(json& j, const Meta_pid::Audio_pid&);
    void to_json(json& j, const Meta_pid&);
    void to_json(json& j, const Meta_channel&);
    void to_json(json& j, const Metadata&);

};

#endif /* METADATA_H */
