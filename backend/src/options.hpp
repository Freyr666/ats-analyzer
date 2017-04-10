#ifndef OPTIONS_H
#define OPTIONS_H

#include <vector>
#include <string>
#include <gstreamermm.h>

#include "chatterer.hpp"
#include "metadata.hpp"

using namespace std;
using namespace Glib;

namespace Ats {

    class Graph;
    class Probe;

    class Options : public Chatterer {

    public:

        class Serializer_buffer_overflow : std::exception {};

        /* ------- Prog list -------------------- */
        vector<Metadata> data;

        /* ------- Mosaic settings -------------- */
        pair<uint,uint> resolution = make_pair(1920, 1080);
        uint background_color = 0;

        sigc::signal<void,const Options&>   set;
        sigc::signal<void,const Options&>   destructive_set;
        sigc::signal<void>                  updated;
    
        Options() {}
        virtual ~Options() {}

        bool   is_empty () const;
        void   set_data(const Metadata&);
        void   set_pid(const uint, const uint, const uint, Meta_pid::Pid_type);
        Metadata*           find_stream (uint stream);
        const Metadata*     find_stream (uint stream) const;

        // Chatter implementation
        string to_string() const;	
        string to_json()   const;
        void   of_json(const string&);
        string to_msgpack()   const;
        void   of_msgpack(const string&);

        void operator=(const Metadata& m) { set_data(m); }

        void   connect(Probe&);
        void   connect(Graph&);
    };
};

#endif /* OPTIONS_H */
