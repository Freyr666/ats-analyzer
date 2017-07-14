#ifndef GRAPH_H
#define GRAPH_H

#include <string>
#include <gstreamermm.h>
#include <glibmm.h>
#include <functional>
#include <map>
#include <boost/variant.hpp>

#include "chatterer.hpp"
#include "metadata.hpp"
#include "wm.hpp"

using namespace std;
using namespace Glib;

namespace Ats {

    class Options;
    class Settings;
    
    class Graph : public Chatterer, public Logger {
	
    public:

        Graph(const std::string& n) : Chatterer(n) {}
        Graph(const Graph&) = delete;
        Graph(Graph&&) = delete;
        virtual ~Graph() {}

	Wm&        get_wm() { return wm; };
        void       set(const Options&);
        void       reset();
        void       apply_options(const Options&);
        void       apply_settings(const Settings&);
	
        void       set_state(Gst::State);
        Gst::State get_state() const;

        void   set_settings(const Settings&);

        void   connect(Options& o);
        void   connect(Settings& o);

        sigc::signal<void,const uint,const uint,const uint,Meta_pid::Pid_type>   set_pid;
	
        // Chatterer
        string to_string() const;
        json   serialize() const;
        void   deserialize(const json&);
	
    private:
	Wm                    wm;
        RefPtr<Gst::Pipeline> pipe;
        RefPtr<Gst::Bus>      bus;
	
        RefPtr<Gst::Bin> create_root(const Metadata&);
        RefPtr<Gst::Bin> create_branch(const uint,
                                       const uint,
                                       const uint,
                                       const Metadata&);
	/* auxilary fun-s */
	void      build_root(const Metadata&,RefPtr<Gst::Bin>,RefPtr<Gst::Element>,const Meta_channel&);
        void      build_branch(const Metadata&,RefPtr<Gst::Bin>,uint,const RefPtr<Gst::Pad>&);
        void      build_subbranch(RefPtr<Gst::Bin>,uint,uint,uint,const RefPtr<Gst::Pad>&);

        bool             on_bus_message(const Glib::RefPtr<Gst::Bus>&,
                                        const Glib::RefPtr<Gst::Message>&);
    };

};

#endif /* GRAPH_H */
