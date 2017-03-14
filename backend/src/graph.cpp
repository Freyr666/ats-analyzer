#include "graph.hpp"
#include "address.hpp"

#include <iostream>
#include <cstdlib> //strtoul
#include <vector>
#include <gstreamermm/tee.h>
#include <gst/gst.h>

using namespace std;
using namespace Ats;

void
Graph::apply(const Options& o) {
    if (o.data[0].channels[0].service_name.empty()) return;
    cout << o.to_string() << endl;
    
    reset();
	
    pipe = Gst::Pipeline::create();
 
    auto mixer  = Gst::ElementFactory::create_element("glvideomixer");
    auto output = Gst::ElementFactory::create_element("glimagesink");

    pipe->add(mixer)->add(output);
    mixer->link(output);
    
    for_each(o.data.begin(),o.data.end(),[this, mixer](const Metadata& m){
	    auto root = create_root(m);

	    if (root) {
		pipe->add(root);
		root->sync_state_with_parent();
		
		root->signal_pad_added().connect([this, mixer](const RefPtr<Gst::Pad>& p) {
			
			auto pname = p->get_name();
			cout << "Caps: " << pname << "\n";
			vector<Glib::ustring> name_toks = Glib::Regex::split_simple("_", pname);
			auto type    = name_toks[1];
			
			if (type == "video") {
			    pipe->set_state(Gst::STATE_PAUSED);
			    auto mixer_pad = mixer->get_request_pad("sink_%u");
			    p->link(mixer_pad);
			    pipe->set_state(Gst::STATE_PLAYING);
			}
		    });
	    }
	});

    ((RefPtr<Gst::Bin>) pipe)->signal_element_added().connect([this](const RefPtr<Gst::Element>&){
	    pipe->set_state(Gst::STATE_PLAYING);
	    gst_bin_sync_children_states(GST_BIN(pipe->gobj()));
	    //   GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN(pipe->gobj()), GST_DEBUG_GRAPH_SHOW_ALL, "pipeline");
	});
    
    bus = pipe->get_bus();
   
    pipe->set_state(Gst::STATE_PLAYING);

    // GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN(pipe->gobj()), GST_DEBUG_GRAPH_SHOW_ALL, "pipeline");
}

void
Graph::reset() {
    if (bus)   bus.reset();
    if (pipe)  {
	set_state(Gst::STATE_NULL);
	pipe.reset();
    }
}

void
Graph::set_state(Gst::State s) {
    pipe->set_state(s);
}

string
Graph::to_string() const {
    return "todo";
}

RefPtr<Gst::Bin>
Graph::create_root(const Metadata& m) {
    if (! m.to_be_analyzed()) return RefPtr<Gst::Bin>(nullptr);
    
    address a = get_address(m.stream);

    auto bin   = Gst::Bin::create();
    auto src   = Gst::ElementFactory::create_element("udpsrc");
    auto parse = Gst::ElementFactory::create_element("tsparse");
    auto tee   = Gst::Tee::create();

    src->set_property("address", a.addr);
    src->set_property("port", a.port);

    bin->add(src)->add(parse)->add(tee);

    src->link(parse)->link(tee);

    m.for_analyzable ([&m,&bin,tee](const Meta_channel& c) {
	    uint num = c.number;
	    
	    string demux_name = "demux_";
	    demux_name += std::to_string(m.stream);
	    demux_name += "_";
	    demux_name += std::to_string(c.number);
	    cout << "Demux name: " << demux_name << endl;

	    auto queue = Gst::ElementFactory::create_element("queue2");
	    auto demux = Gst::ElementFactory::create_element("tsdemux",demux_name);

	    demux->set_property("program-number", c.number);
	    
	    auto sinkpad = queue->get_static_pad("sink"); 
	    auto srcpad = tee->get_request_pad("src_%u");

	    bin->add(queue)->add(demux);
	    queue->link(demux);

	    srcpad->link(sinkpad);
	    
	    demux->signal_pad_added().connect([&m, bin, num](const RefPtr<Gst::Pad>& p) {
		    if (p->is_ghost_pad()) return;
		    auto pname = p->get_name();
		    auto pcaps = p->get_current_caps()->get_structure(0).get_name();
        
		    vector<Glib::ustring> name_toks = Glib::Regex::split_simple("_", pname);
		    vector<Glib::ustring> caps_toks = Glib::Regex::split_simple("/", pcaps);
		    
		    auto type    = caps_toks[0];
		    auto subtype = caps_toks[1];
		    
		    auto pid  = strtoul(name_toks[2].data(), NULL, 16);

		    auto branch = create_branch(num, pid, type, subtype, m);

		    if (!branch) return;

		    string src_pad_name = "src_";
		    src_pad_name += type + "_";
		    src_pad_name += std::to_string(m.stream);
		    src_pad_name += "_";
		    src_pad_name += std::to_string(num);
		    src_pad_name += "_";
		    src_pad_name += std::to_string(pid);

		    bin->add(branch);
		    bin->set_state(Gst::STATE_PAUSED);
		    auto sink_pad  = branch->get_static_pad("sink");
		    p->link(sink_pad);

		    if (type == "video") {
			auto src_pad   = branch->get_static_pad("src");
			auto src_ghost = Gst::GhostPad::create(src_pad, src_pad_name);
			src_ghost->set_active();
			bin->add_pad(src_ghost);
		    }
		    bin->set_state(Gst::STATE_PLAYING);
		    branch->sync_state_with_parent();
		});
	    
	});
    return bin;
}

RefPtr<Gst::Bin>
Graph::create_branch(const uint channel,
		     const uint pid,
		     const string type,
		     const string subtype,
		     const Metadata& m) {
    RefPtr<Gst::Element> decoder;
    RefPtr<Gst::Element> deint;
    RefPtr<Gst::Pad> src_pad;
    
    auto pidinfo = m.find_pid(channel, pid);

    if (pidinfo == nullptr) return RefPtr<Gst::Bin>(nullptr);
    
    auto bin = Gst::Bin::create();
    auto queue  = Gst::ElementFactory::create_element("queue");
    
    // TODO: switch to decodebin    
    if (type == "video") {
	
	RefPtr<Gst::Element> parser;
	
	if (subtype == "x-h264") {
	    parser  = Gst::ElementFactory::create_element("h264parse");
	    decoder = Gst::ElementFactory::create_element("avdec_h264");
	} else if (subtype == "mpeg") {
	    parser  = Gst::ElementFactory::create_element("mpegvideoparse");
	    decoder = Gst::ElementFactory::create_element("avdec_mpeg2video");
	} else {
	    return RefPtr<Gst::Bin>(nullptr);
	}
	deint = Gst::ElementFactory::create_element("deinterlace");
	// TODO: add analyzer
	bin->add(queue)->add(parser)->add(decoder)->add(deint);
	queue->link(parser)->link(decoder)->link(deint);
	
    } else if (type == "audio_no") {
	
	RefPtr<Gst::Element> decoder;	
        bin->add(queue);
	if (subtype == "x-ac3") {
	    decoder = Gst::ElementFactory::create_element("avdec_ac3_fixed");
	    bin->add(decoder);
	    queue->link(decoder);
	} else if (subtype == "x-eac3") {
	    decoder = Gst::ElementFactory::create_element("avdec_eac3");
	    bin->add(decoder);
	    queue->link(decoder);
	} else if (subtype == "mpeg") {
	    auto parser = Gst::ElementFactory::create_element("mpegaudioparse");
	    decoder = Gst::ElementFactory::create_element("mpg123audiodec");
	    bin->add(parser)->add(decoder);
	    queue->link(parser)->link(decoder);
	} else {
	    return RefPtr<Gst::Bin>(nullptr);
	}
    } else {
	return RefPtr<Gst::Bin>(nullptr);
    }
    auto sink_pad = queue->get_static_pad("sink");
    if (type == "video") {
	src_pad  = deint->get_static_pad("src");
    } else {
	src_pad  = decoder->get_static_pad("src");
    }
    
    auto sink_ghost = Gst::GhostPad::create(sink_pad, "sink");
    auto src_ghost = Gst::GhostPad::create(src_pad, "src");

    sink_ghost->set_active();
    src_ghost->set_active();

    bin->add_pad(sink_ghost);
    bin->add_pad(src_ghost);

    return bin;
}
