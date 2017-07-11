#include <cstdlib> //strtoul
#include <vector>
#include <gstreamermm/tee.h>
#include <gst/gst.h>
#include <gst/video/video.h>

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
			    // auto channel = strtoul(name_toks[3].c_str(), NULL, 10);
			    auto pid     = strtoul(name_toks[4].c_str(), NULL, 10);

			    this->wm.add_sink(m.stream, pid, type, p);
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

    m.for_analyzable ([this,&m,&bin,tee](const Meta_channel& c) {
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
	    
            demux->signal_pad_added().connect([this,&m, bin, num](const RefPtr<Gst::Pad>& p) {
		    auto stream = m.stream;
		    
                    auto pname = p->get_name();
                    auto pcaps = p->get_current_caps()->get_structure(0).get_name();
        
                    vector<Glib::ustring> name_toks = Glib::Regex::split_simple("_", pname);
                    vector<Glib::ustring> caps_toks = Glib::Regex::split_simple("/", pcaps);

                    auto& type = caps_toks[0];
		    
                    if (type != "video" && type != "audio") return;	    
		    
                    auto pid  = strtoul(name_toks[2].data(), NULL, 16);

                    auto branch = create_branch(stream, num, pid, m);
		    
                    if (!branch) return;

                    bin->add(branch);
                    auto sink_pad  = branch->get_static_pad("sink");
                    p->link(sink_pad);

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
Graph::create_branch(const uint stream,
                     const uint channel,
                     const uint pid,
                     const Metadata& m) {
/*
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

    decoder->signal_pad_added().connect([this,bin,stream,channel,pid](const RefPtr<Gst::Pad>& p) {
	    RefPtr<Gst::Pad> src_pad;
	    Graph::Node n;

	    auto set_video = [this,&p,stream,channel,pid](){
		Meta_pid::Video_pid v;

		auto vi = gst_video_info_new();
		auto pcaps = p->get_current_caps();
		if (! gst_video_info_from_caps(vi, pcaps->gobj())) {
		    gst_video_info_free(vi);
		    return;
		}
			
		v.codec = "h264"; // FIXME not only h264 supported
		v.width = vi->width;
		v.height = vi->height;
		v.aspect_ratio = {vi->par_n,vi->par_d};
		v.frame_rate = (float)vi->fps_n/vi->fps_d;

		gst_video_info_free(vi);

		Meta_pid::Pid_type rval = v;

		set_pid.emit(stream,channel,pid,rval);
	    };
	    
            auto pcaps = p->get_current_caps()->get_structure(0).get_name();
            vector<Glib::ustring> caps_toks = Glib::Regex::split_simple("/", pcaps);
            auto& type    = caps_toks[0];
	    

            if (type == "video") {
		// Video callback
		p->connect_property_changed("caps", set_video);
		set_video();
		
                auto _deint  = Gst::ElementFactory::create_element("deinterlace");
		auto anal    = Gst::ElementFactory::create_element("videoanalysis");
		auto _scale  = Gst::ElementFactory::create_element("videoscale");
		auto _caps   = Gst::ElementFactory::create_element("capsfilter");

		_caps->set_property("caps", Gst::Caps::create_from_string("video/x-raw,pixel-aspect-ratio=1/1"));
	      
		n.type = type;
		n.analysis = anal;
		
		this->elms.add(stream,channel,pid,n);
		
                auto _sink = _deint->get_static_pad("sink");
		src_pad = _caps->get_static_pad("src");
		
                bin->add(_deint)->add(anal)->add(_scale)->add(_caps);
		_deint->link(anal)->link(_scale)->link(_caps);
		
		_deint->sync_state_with_parent();
		anal->sync_state_with_parent();
		_scale->sync_state_with_parent();
		_caps->sync_state_with_parent();
		
                p->link(_sink);
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
*/
    return RefPtr<Gst::Bin>(nullptr);
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
