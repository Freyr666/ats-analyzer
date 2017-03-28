#include <iostream>
#include <cstdlib> //strtoul
#include <vector>
#include <gstreamermm/tee.h>
#include <gst/gst.h>

#include "graph.hpp"
#include "address.hpp"
#include "json.hpp"

using namespace std;
using namespace Ats;

static inline Gst::State
to_state(string& s) {
    if (s == "null") return Gst::STATE_NULL;
    if (s == "pause") return Gst::STATE_PAUSED;
    if (s == "play") return Gst::STATE_PLAYING;
    if (s == "stop") return Gst::STATE_NULL;
    // FIXME
    return Gst::STATE_NULL;
}

void
Graph::set(const Options& o) {
    if (o.is_empty()) return;
    
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
                        //cout << "Caps: " << pname << "\n";
                        vector<Glib::ustring> name_toks = Glib::Regex::split_simple("_", pname);
                        auto type    = name_toks[1];
			
                        if (type == "video") {
                            auto mixer_pad = mixer->get_request_pad("sink_%u");
                            p->link(mixer_pad);
                        }
                    });
            }
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
Graph::apply_options(const Options&) {

}

void
Graph::apply_settings(const Settings&) {

}

void
Graph::set_state(Gst::State s) {
    if (pipe)
	pipe->set_state(s);
}

Gst::State
Graph::get_state() {
    Gst::State rval;
    Gst::State pend;
    if (pipe) {
	pipe->get_state(rval, pend, Gst::MICRO_SECOND);
    } else {
	rval = Gst::STATE_NULL;
    }
    return rval;
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

    m.for_analyzable ([&m,&bin,tee](const Meta_channel& c) {
            uint num = c.number;
	    
            string demux_name = "demux_";
            demux_name += std::to_string(m.stream);
            demux_name += "_";
            demux_name += std::to_string(c.number);
            //cout << "Demux name: " << demux_name << endl;

            auto queue = Gst::ElementFactory::create_element("queue2");
            auto demux = Gst::ElementFactory::create_element("tsdemux",demux_name);

            demux->set_property("program-number", c.number);
            queue->set_property("max-size-buffers", 200000);
            queue->set_property("max-size-bytes", 429496729);

            auto sinkpad = queue->get_static_pad("sink"); 
            auto srcpad = tee->get_request_pad("src_%u");

            bin->add(queue)->add(demux);
            queue->link(demux);

            srcpad->link(sinkpad);
	    
            demux->signal_pad_added().connect([&m, bin, num](const RefPtr<Gst::Pad>& p) {	    
                    auto pname = p->get_name();
                    auto pcaps = p->get_current_caps()->get_structure(0).get_name();
        
                    vector<Glib::ustring> name_toks = Glib::Regex::split_simple("_", pname);
                    vector<Glib::ustring> caps_toks = Glib::Regex::split_simple("/", pcaps);

                    auto& type = caps_toks[0];
		    
                    if (type != "video" && type != "audio") return;	    
		    
                    auto pid  = strtoul(name_toks[2].data(), NULL, 16);

                    auto branch = create_branch(num, pid, m);
		    
                    if (!branch) return;

                    bin->add(branch);
                    auto sink_pad  = branch->get_static_pad("sink");
                    p->link(sink_pad);

                    auto stream = m.stream;

                    branch->signal_pad_added().connect([bin, type, pid, num, stream](const RefPtr<Gst::Pad>& p) {
                            if (type != "video") return;
			    
                            string src_pad_name = "src_";
                            src_pad_name += type + "_";
                            src_pad_name += std::to_string(stream);
                            src_pad_name += "_";
                            src_pad_name += std::to_string(num);
                            src_pad_name += "_";
                            src_pad_name += std::to_string(pid);

                            auto src_ghost = Gst::GhostPad::create(p, src_pad_name);
                            src_ghost->set_active();
                            bin->add_pad(src_ghost);
                        });
                    branch->sync_state_with_parent();
                });
	    
        });
    return bin;
}

RefPtr<Gst::Bin>
Graph::create_branch(const uint channel,
                     const uint pid,
                     const Metadata& m) {
    RefPtr<Gst::Pad> src_pad;
    auto pidinfo = m.find_pid(channel, pid);

    if (pidinfo == nullptr) return RefPtr<Gst::Bin>(nullptr);
    
    auto bin     = Gst::Bin::create();
    auto queue   = Gst::ElementFactory::create_element("queue");
    auto decoder = Gst::ElementFactory::create_element("decodebin");
    
    queue->set_property("max-size-buffers", 20000);
    queue->set_property("max-size-bytes", 12000000);

    bin->add(queue)->add(decoder);
    queue->link(decoder);

    decoder->signal_pad_added().connect([bin](const RefPtr<Gst::Pad>& p) {
            RefPtr<Gst::Pad> src_pad;
	    
            auto pcaps = p->get_current_caps()->get_structure(0).get_name();
            vector<Glib::ustring> caps_toks = Glib::Regex::split_simple("/", pcaps);
            auto& type    = caps_toks[0];

            if (type == "video") {		
                auto deint = Gst::ElementFactory::create_element("deinterlace");
                auto deint_sink = deint->get_static_pad("sink");
                src_pad = deint->get_static_pad("src");
                bin->add(deint);
                deint->sync_state_with_parent();
                p->link(deint_sink);
            } else if (type == "audio") {
                src_pad = p;
            } else {
                return;
            }
            auto src_ghost = Gst::GhostPad::create(src_pad, "src");
            src_ghost->set_active();
            bin->add_pad(src_ghost);
        });

    auto sink_pad = queue->get_static_pad("sink");
    auto sink_ghost = Gst::GhostPad::create(sink_pad, "sink");

    sink_ghost->set_active();
    bin->add_pad(sink_ghost);

    return bin;
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

// Chatterer

string
Graph::to_string() const {
    return "todo";
}

string
Graph::to_json() const {
    return "todo";
}

string
Graph::to_msgpack() const {
    return "todo";
}

void
Graph::of_json(const string& j) {
    using json = nlohmann::json;
    auto js = json::parse(j);
 
    // TODO throw Wrong_json
    if (! js.is_object()) return;

    for (json::iterator el = js.begin(); el != js.end(); ++el) {
 	if (el.key() == "state") {
	    auto sst = el.value().get<string>();
	    auto st = to_state(sst);
	    set_state(st);
	} 
    }
}

void
Graph::of_msgpack(const string&) {
    
}
