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
	
    pipe = Gst::Pipeline::create();

    wm.init(pipe);
 
    auto output = Gst::ElementFactory::create_element("glimagesink");
    pipe->add(output);

    wm.get_src()->link(output->get_static_pad("sink"));

    for_each(o.data.begin(),o.data.end(),[this](const Metadata& m){
            auto root = create_root(m);

            if (root) {
                pipe->add(root);
                root->sync_state_with_parent();
		
                root->signal_pad_added().connect([this, m](const RefPtr<Gst::Pad>& p) {
			auto pname = p->get_name();
                        
			vector<Glib::ustring> name_toks = Glib::Regex::split_simple("_", pname);
			auto type    = name_toks[1];

			if (type == "video") {
			    //auto channel = strtoul(name_toks[3].c_str(), NULL, 10);
			    auto pid     = strtoul(name_toks[4].c_str(), NULL, 10);

			    this->wm.add_sink(m.stream, pid, type, *m.find_pid(pid), p);
			}
			
		    });
            }
        });

    bus = pipe->get_bus();

    bus->add_watch(sigc::mem_fun(this, &Graph::on_bus_message));
   
    pipe->set_state(Gst::STATE_PLAYING);

    // GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN(pipe->gobj()), GST_DEBUG_GRAPH_SHOW_ALL, "pipeline");
}

void
Graph::reset() {
    if (bus)   bus.reset();
    if (pipe)  {
	set_state(Gst::STATE_PAUSED);
	set_state(Gst::STATE_NULL);
        pipe.reset();
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
    if (pipe)
	pipe->set_state(s);
}

Gst::State
Graph::get_state() const {
    Gst::State rval;
    Gst::State pend;
    if (pipe) {
	pipe->get_state(rval, pend, Gst::MICRO_SECOND);
    } else {
	rval = Gst::STATE_NULL;
    }
    return rval;
}

void
Graph::set_settings(const Settings&) {
    
}

RefPtr<Gst::Bin>
Graph::create_root(const Metadata& m) {
    if (! m.to_be_analyzed()) return RefPtr<Gst::Bin>(nullptr);

    auto bin   = Gst::Bin::create();
    auto src   = Gst::ElementFactory::create_element("udpsrc");
    auto parse = Gst::ElementFactory::create_element("tsparse");
    auto tee   = Gst::Tee::create();

    src->set_property("uri", m.uri);
    src->set_property("buffer-size", 2147483647);

    bin->add(src)->add(parse)->add(tee);

    src->link(parse)->link(tee);

    m.for_analyzable ([this,&m,bin,tee](const Meta_channel& c) { build_root(m,bin,tee,c); });
    return bin;
}

RefPtr<Gst::Bin>
Graph::create_branch(const uint stream,
                     const uint channel,
                     const uint pid,
                     const Metadata& m) {

    RefPtr<Gst::Pad> src_pad;
    
    auto pidinfo = m.find_pid(channel, pid);

    if ((pidinfo == nullptr) || (!pidinfo->to_be_analyzed)) return RefPtr<Gst::Bin>(nullptr);
    
    auto bin     = Gst::Bin::create();
    auto queue   = Gst::ElementFactory::create_element("queue");
    auto decoder = Gst::ElementFactory::create_element("decodebin");
    
    queue->set_property("max-size-buffers", 20000);
    queue->set_property("max-size-bytes", 12000000);

    bin->add(queue)->add(decoder);
    queue->link(decoder);

    decoder->signal_pad_added().connect([this,bin,stream,channel,pid](const RefPtr<Gst::Pad>& pad)
					{ build_subbranch(bin,stream,channel,pid,pad); });

    auto sink_pad = queue->get_static_pad("sink");
    auto sink_ghost = Gst::GhostPad::create(sink_pad, "sink");

    sink_ghost->set_active();
    bin->add_pad(sink_ghost);

    return bin;
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
