#include <glibmm.h>
#include <gst/gst.h>
#include <gst/video/video.h>

#include "branch.hpp"

using namespace std;
using namespace Ats;

unique_ptr<Branch>
Branch::create(std::string type, uint stream, uint chan, uint pid) {
    if (type == "video") return unique_ptr<Branch>((Branch*) new Video_branch(stream, chan, pid));
    else if (type == "audio") return unique_ptr<Branch>((Branch*) new Audio_branch(stream, chan, pid));
    else return unique_ptr<Branch>(nullptr);
}

Branch::Branch() {
    _bin    = Gst::Bin::create();
    auto queue   = Gst::ElementFactory::create_element("queue");
    _decoder = Gst::ElementFactory::create_element("decodebin");

    queue->set_property("max-size-buffers", 20000);
    queue->set_property("max-size-bytes", 12000000);

    _bin->add(queue)->add(_decoder);
    queue->link(_decoder);

    auto p = queue->get_static_pad("sink");
    auto sink_ghost = Gst::GhostPad::create(p, "sink");
    sink_ghost->set_active();
    _bin->add_pad(sink_ghost);
}

Branch::~Branch() {
    // _bin->unparent();
}
    
void
Branch::plug ( const Glib::RefPtr<Gst::Pad>& p ) {
    auto sink_pad  = _bin->get_static_pad("sink");
    p->link(sink_pad);
    _bin->sync_state_with_parent();
}

Video_branch::Video_branch(uint stream, uint chan, uint pid) : Branch () {
    _stream = stream;
    _channel = chan;
    _pid = pid;

    _decoder->signal_pad_added().
	connect([this,stream,chan,pid](const Glib::RefPtr<Gst::Pad>& pad)
		{
		    auto pcaps = pad->get_current_caps()->get_structure(0).get_name();
		    vector<Glib::ustring> caps_toks = Glib::Regex::split_simple("/", pcaps);
		    auto& type    = caps_toks[0];		    

		    if (type != "video") return;

		    pad->connect_property_changed("caps", [this, pad](){ set_video(pad); });

		    auto deint  = Gst::ElementFactory::create_element("deinterlace");
		    auto _analyser = Gst::ElementFactory::create_element("videoanalysis");
		    
		    auto sink_pad = deint->get_static_pad("sink");
		    auto src_pad  = _analyser->get_static_pad("src");

		    _bin->add(deint)->add(_analyser);
		    deint->link(_analyser);

                    deint->sync_state_with_parent();
                    _analyser->sync_state_with_parent();

		    pad->link(sink_pad);

		    auto src_ghost = Gst::GhostPad::create(src_pad, "src");
		    src_ghost->set_active();
		    _bin->add_pad(src_ghost);

		    auto p = shared_ptr<Pad>(new Pad(stream,
						     chan, pid,
						     "video",
						     (Glib::RefPtr<Gst::Pad>) src_ghost));

		    _pads.push_back(p);
		    _pad_added.emit(p);
		});
    
}

void
Video_branch::set_video (const Glib::RefPtr<Gst::Pad> p) {
    Meta_pid::Video_pid v;

    if (! p) return;

    auto vi = gst_video_info_new();
    auto pcaps = p->get_current_caps();
    if (! pcaps) return;
    
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

    _set_pid.emit(_stream,_channel,_pid,rval);
}

Audio_branch::Audio_branch(uint stream, uint chan, uint pid) {

}
