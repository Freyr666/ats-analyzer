#include <cstdlib> //strtoul
#include <vector>
#include <gstreamermm/tee.h>

#include "graph.hpp"
#include "streams.hpp"
#include "settings.hpp"

using namespace std;
using namespace Ats; 



void
Graph::set(const Streams& o) {   
    reset();
    
    if (o.is_empty()) {      
        return;
    }

    _pipe = Gst::Pipeline::create();   
    _wm.add_to_pipe(_pipe);

    _vrenderer.reset(new Video_renderer(5004));
    _vrenderer->add_to_pipe(_pipe);
    _vrenderer->plug(_wm);
    
    for_each(o.data.begin(),o.data.end(),[this](const Metadata& m){
            // TODO separate create and add_to_pipe
            auto root = Root::create(_pipe, m, _settings, _video_sender, _audio_sender);
            if (root) {		
                root->signal_pad_added().connect([this, m](std::shared_ptr<Pad> p) {
                        std::scoped_lock lock{_mutex_pad};
                        _wm.plug(p);
			GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN(_pipe->gobj()), GST_DEBUG_GRAPH_SHOW_ALL, "pipeline");
                    });
                root->signal_audio_pad_added().connect([this, m](std::shared_ptr<Pad> p) {
                        std::scoped_lock lock{_mutex_audio_pad}; /* TODO FIX */
                        auto ar = unique_ptr<Audio_renderer> (new Audio_renderer (5005 + p->stream() + p->pid()));
                        ar->add_to_pipe (_pipe);
                        ar->plug (p);
                        _arenderers.push_back(std::move(ar));
                        //GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN(_pipe->gobj()), GST_DEBUG_GRAPH_SHOW_ALL, "pipeline");
                    });
                _roots.push_back(std::move(root));
            }
        });

    _bus = _pipe->get_bus();

    _bus->add_watch(sigc::mem_fun(this, &Graph::on_bus_message));
   
    _pipe->set_state(Gst::STATE_PLAYING);

    // GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN(_pipe->gobj()), GST_DEBUG_GRAPH_SHOW_ALL, "pipeline");
}

void
Graph::reset() {
    if (_bus)   _bus.reset();
    if (_pipe)  {
	set_state(Gst::State::STATE_PAUSED);
	set_state(Gst::State::STATE_NULL);
        _wm.reset();
        _vrenderer.reset();
        _arenderers.clear();        
        _roots.clear();
        //GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN(_pipe->gobj()), GST_DEBUG_GRAPH_SHOW_ALL, "pipeline");
        _pipe.reset();
    }   
}

void
Graph::apply_streams(const Streams&) {

}

void
Graph::apply_settings(const Settings& s) {
    _settings = s;
    for (auto& root : _roots) {
        root->apply(_settings);
    }
}

void
Graph::set_state(Gst::State s) {
    if (_pipe)
	_pipe->set_state(s);
}

Gst::State
Graph::get_state() const {
    Gst::State rval;
    Gst::State pend;
    if (_pipe) {
	_pipe->get_state(rval, pend, Gst::MICRO_SECOND);
    } else {
	rval = Gst::STATE_NULL;
    }
    return rval;
}

#define DATA_MARKER 0x8BA820F0
#include <iostream>

bool
Graph::on_bus_message(const Glib::RefPtr<Gst::Bus>& bus,
                      const Glib::RefPtr<Gst::Message>& msg) {
    return    false;
}

void
Graph::connect(Streams& o) {
    o.destructive_set.connect(sigc::mem_fun(this, &Graph::set));
    o.set.connect(sigc::mem_fun(this, &Graph::apply_streams));
}

void
Graph::connect(Settings_facade& s) {
    s.set.connect(sigc::mem_fun(this, &Graph::apply_settings));
}
