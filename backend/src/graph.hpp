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
#include "root.hpp"
#include "renderer.hpp"

namespace Ats {

    class Options;
    class Settings;
    
    class Graph : public Chatterer, public Logger {
	
    public:

        Graph(const std::string& n) : Chatterer(n) {}
        Graph(const Graph&) = delete;
        Graph(Graph&&) = delete;
        virtual ~Graph() {}

	Wm&        get_wm() { return _wm; };
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
	Wm                                 _wm;
	Video_renderer                     _vrenderer;
	std::vector<std::unique_ptr<Audio_renderer>> _arenderers;
	std::vector<std::unique_ptr<Root>> _roots;
	Glib::RefPtr<Gst::Pipeline>        _pipe;
	Glib::RefPtr<Gst::Bus>             _bus;

        bool             on_bus_message(const Glib::RefPtr<Gst::Bus>&,
                                        const Glib::RefPtr<Gst::Message>&);
    };

};

#endif /* GRAPH_H */
