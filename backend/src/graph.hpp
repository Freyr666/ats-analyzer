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
#include "video_data.hpp"

namespace Ats {

    class Streams;
    class Settings;
    
    class Graph : public Chatterer, public Logger {
	
    public:
        
        Graph(const std::string& n) : Chatterer(n),
                                      _video_sender(shared_ptr<Video_data>(new Video_data())),
                                      _vrenderer(unique_ptr<Video_renderer>(new Video_renderer())) {}
        Graph(const Graph&) = delete;
        Graph(Graph&&) = delete;
        virtual ~Graph() {}

        Wm&        get_wm() { return _wm; };
        Video_data& get_video_sender() { return *_video_sender; }
        void       set(const Streams&);
        void       reset();
        
        void       apply_streams(const Streams&);
        void       apply_settings(const Settings&);
	
        void       set_state(Gst::State);
        Gst::State get_state() const;

        void   connect(Streams& o);
        void   connect(Settings_facade& o);

        sigc::signal<void,const uint,const uint,const uint,Meta_pid::Pid_type>   set_pid;
	
        // Chatterer
        string to_string() const;
        json   serialize() const;
        void   deserialize(const json&);
	
    private:
        Settings                           _settings;
        Wm                                 _wm;
        std::shared_ptr<Video_data>        _video_sender;
        std::unique_ptr<Video_renderer>    _vrenderer;
        std::vector<std::unique_ptr<Audio_renderer>> _arenderers;
        std::vector<std::unique_ptr<Root>> _roots;
        Glib::RefPtr<Gst::Pipeline>        _pipe;
        Glib::RefPtr<Gst::Bus>             _bus;

        bool             on_bus_message(const Glib::RefPtr<Gst::Bus>&,
                                        const Glib::RefPtr<Gst::Message>&);
    };

};

#endif /* GRAPH_H */
