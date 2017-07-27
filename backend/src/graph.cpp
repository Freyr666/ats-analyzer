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

    _wm.init(_pipe);
 
    auto output = Gst::ElementFactory::create_element("glimagesink");
    _pipe->add(output);

    _wm.get_src()->link(output->get_static_pad("sink"));

    for_each(o.data.begin(),o.data.end(),[this](const Metadata& m){
            auto root = Root::create(_pipe, m);

            if (root) {
		
                root->signal_pad_added().connect([this, m](std::shared_ptr<Pad> p) {

			if (p->type() == Pad::Type::Video) {
			    //auto channel = strtoul(name_toks[3].c_str(), NULL, 10);
			    // auto pid   = 

			    // this->_wm.add_sink(m.stream, pid, type, *m.find_pid(pid), p);
			}
			
		    });
		_roots.push_back(std::move(root));
            }
        });

    _bus = _pipe->get_bus();

    _bus->add_watch(sigc::mem_fun(this, &Graph::on_bus_message));
   
    _pipe->set_state(Gst::STATE_PLAYING);

    // GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN(pipe->gobj()), GST_DEBUG_GRAPH_SHOW_ALL, "pipeline");
}

void
Graph::reset() {
    if (_bus)   _bus.reset();
    if (_pipe)  {
	set_state(Gst::STATE_PAUSED);
	set_state(Gst::STATE_NULL);
        _pipe.reset();
    }
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
