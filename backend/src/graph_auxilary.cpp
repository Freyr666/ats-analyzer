#include "graph.hpp"
#include <gstreamermm/tee.h>
#include <gst/gst.h>
#include <gst/video/video.h>

using namespace std;
using namespace Ats; 

void
Graph::build_root(const Metadata& m, RefPtr<Gst::Bin> bin, RefPtr<Gst::Element> tee, const Meta_channel& c) {
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

    demux->signal_pad_added().connect([this,&m, bin, num](const RefPtr<Gst::Pad>& p)
				      {  build_branch( m, bin, num, p); });
}

void
Graph::build_branch(const Metadata& m, RefPtr<Gst::Bin> bin, uint num, const RefPtr<Gst::Pad>& p) {
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
}

void
Graph::build_subbranch(RefPtr<Gst::Bin> bin, uint stream, uint channel, uint pid, const RefPtr<Gst::Pad>& p) {
    RefPtr<Gst::Pad> src_pad;

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

	/*
	n.type = type;
	n.analysis = anal;
	*/
			
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
}
