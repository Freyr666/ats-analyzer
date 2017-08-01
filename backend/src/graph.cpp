#include <cstdlib> //strtoul
#include <vector>
#include <gstreamermm/tee.h>

#include "graph.hpp"
#include "options.hpp"
#include "settings.hpp"

using namespace std;
using namespace Ats; 

void
Graph::set(const Options& o) {
    if (o.is_empty()) return;
    
    reset();
	
    _pipe = Gst::Pipeline::create();

    _wm.add_to_pipe(_pipe);
    _vrenderer.add_to_pipe(_pipe);

    _vrenderer.plug(_wm);

    for_each(o.data.begin(),o.data.end(),[this](const Metadata& m){
            // TODO separate create and add_to_pipe
            auto root = Root::create(_pipe, m);

            if (root) {		
                root->signal_pad_added().connect([this, m](std::shared_ptr<Pad> p) {
                        _wm.plug(p);
			// GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN(_pipe->gobj()), GST_DEBUG_GRAPH_SHOW_ALL, "pipeline");
                    });
                root->signal_audio_pad_added().connect([this, m](std::shared_ptr<Pad> p) {	
                        auto ar = unique_ptr<Audio_renderer> (new Audio_renderer ());
                        ar->add_to_pipe (_pipe);
                        ar->plug (p);
                        _arenderers.push_back(std::move(ar));
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
    if (_pipe)  {
	set_state(Gst::STATE_PAUSED);
	set_state(Gst::STATE_NULL);
        _pipe.reset();
    }
    if (_bus)   _bus.reset();
}

void
Graph::apply_options(const Options&) {
    
}

void
Graph::apply_settings(const Settings& s) {
    set_settings(s);
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

void
Graph::set_settings(const Settings&) {
    
}

bool
Graph::on_bus_message(const Glib::RefPtr<Gst::Bus>& bus,
                      const Glib::RefPtr<Gst::Message>& msg) {
    return    false; // switch to true
}

void
Graph::connect(Options& o) {
    o.destructive_set.connect(sigc::mem_fun(this, &Graph::set));
    o.set.connect(sigc::mem_fun(this, &Graph::apply_options));
}

void
Graph::connect(Settings& s) {
    s.set.connect(sigc::mem_fun(this, &Graph::apply_settings));
}
