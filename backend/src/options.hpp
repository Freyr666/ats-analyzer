#ifndef OPTIONS_H
#define OPTIONS_H

#include <vector>
#include <string>
#include <gstreamermm.h>

#include "chatterer.hpp"
#include "metadata.hpp"
#include "probe.hpp"

using namespace std;
using namespace Glib;

namespace Ats {

    class Options : public Chatterer {

    public:

        class Serializer_buffer_overflow : std::exception {};

        /* ------- Prog list -------------------- */
        vector<Metadata> data;

        /* ------- Mosaic settings -------------- */
        pair<int,int> resolution = make_pair(1920, 1080);
        string background_color = "";

        sigc::signal<void,const Options&>   set;
        sigc::signal<void,const Options&>   destructive_set;
    
        Options() {}
        virtual ~Options() {}

        bool   is_empty () const;
        void   set_data(const Metadata&);

        // Chatter implementation
        string to_string() const;	
        string to_json()   const;
        void   of_json(const string&);
        string to_msgpack()   const;
        void   of_msgpack(const string&);

        void operator=(const Metadata& m) { set_data(m); }

        void   connect(Probe& p) { p.updated.connect(
                sigc::mem_fun(this, &Options::set_data));
        }
    };
};

#endif /* OPTIONS_H */
