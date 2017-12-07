#include <glibmm.h>
#include <gst/gst.h>
#include <gst/video/video.h>

#include "branch.hpp"

using namespace std;
using namespace Ats;

unique_ptr<Branch>
Branch::create(std::string type, uint stream, uint chan, uint pid, std::shared_ptr<Video_data> vd, std::shared_ptr<Audio_data> ad) {
    if (type == "video") return unique_ptr<Branch>((Branch*) new Video_branch(stream, chan, pid, vd));
    else if (type == "audio") return unique_ptr<Branch>((Branch*) new Audio_branch(stream, chan, pid, ad));
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

Branch::~Branch() {}
    
void
Branch::plug ( const Glib::RefPtr<Gst::Pad>& p ) {
    auto sink_pad  = _bin->get_static_pad("sink");
    p->link(sink_pad);
    _bin->sync_state_with_parent();
}

static void
videodata_callback (GstElement*,
                    guint64    ds,
                    GstBuffer* d,
                    guint64    es,
                    GstBuffer* e,
                    gpointer   _branch) {
    Glib::RefPtr<Gst::Buffer> data = Glib::wrap(d);
    Glib::RefPtr<Gst::Buffer> errors = Glib::wrap(e);
    Video_branch* branch = (Video_branch*)_branch;
    branch->parse_data_msg(ds, data, es, errors);
}

Video_branch::Video_branch(uint stream, uint chan, uint pid, std::shared_ptr<Video_data> vs) : Branch () {
    _stream = stream;
    _channel = chan;
    _pid = pid;
    _video_sender = vs;

    _decoder->signal_pad_added().
	connect([this,stream,chan,pid](const Glib::RefPtr<Gst::Pad>& pad) {
                auto pcaps = pad->get_current_caps()->get_structure(0).get_name();
                vector<Glib::ustring> caps_toks = Glib::Regex::split_simple("/", pcaps);
                auto& type    = caps_toks[0];		    

                if (type != "video") return;

                pad->connect_property_changed("caps", [this, pad](){ set_video(pad); });

                auto deint  = Gst::ElementFactory::create_element("deinterlace");
                auto _analyser = Gst::ElementFactory::create_element("videoanalysis");

                if (! deint) Error_expn("Branch: deinterlace is not found");
                if (! _analyser) Error_expn("Branch: videoanalysis is not found");
                    
                g_signal_connect(_analyser->gobj(), "data", G_CALLBACK(videodata_callback), this);

                /* TODO consider placing analyser before deinterlacer */
                deint->set_property("method", 7);
		    
                auto sink_pad = deint->get_static_pad("sink");
                auto src_pad  = _analyser->get_static_pad("src");

                _bin->add(deint)->add(_analyser);
                deint->link(_analyser);

                deint->sync_state_with_parent();
                _analyser->sync_state_with_parent();

                pad->link(sink_pad);

                auto p = shared_ptr<Pad>(new Pad(stream, chan, pid,
                                                 "video", _bin, src_pad));

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
    v.resolution = {vi->width,vi->height};
    v.aspect_ratio = {vi->par_n,vi->par_d};
    v.frame_rate = (float)vi->fps_n/vi->fps_d;

    gst_video_info_free(vi);

    Meta_pid::Pid_type rval = v;

    _set_pid.emit(_stream,_channel,_pid,rval);
}

void
Video_branch::parse_data_msg(int64_t ds, Glib::RefPtr<Gst::Buffer> d,int64_t es, Glib::RefPtr<Gst::Buffer> e) {
    _video_sender->parse_data_msg(_stream, _channel, _pid, ds,d,es,e);
}

void
Video_branch::apply (const Settings& s) {
    const Settings::Video& vs = s.video;

    if (_analyser) {
        _analyser->set_property("loss", vs.loss);
        _analyser->set_property("black-pixel-lb", vs.black.black_pixel);
        _analyser->set_property("pixel-diff-lb", vs.freeze.pixel_diff);
        _analyser->set_property("black-cont", vs.black.black.cont);
        _analyser->set_property("black-cont-en", vs.black.black.cont_en);
        _analyser->set_property("black-peak", vs.black.black.peak);
        _analyser->set_property("black-peak-en", vs.black.black.peak_en);
        _analyser->set_property("black-duration", vs.black.black.duration);
        _analyser->set_property("luma-cont", vs.black.luma.cont);
        _analyser->set_property("luma-cont-en", vs.black.luma.cont_en);
        _analyser->set_property("luma-peak", vs.black.luma.peak);
        _analyser->set_property("luma-peak-en", vs.black.luma.peak_en);
        _analyser->set_property("luma-duration", vs.black.luma.duration);
        _analyser->set_property("freeze-cont", vs.freeze.freeze.cont);
        _analyser->set_property("freeze-cont-en", vs.freeze.freeze.cont_en);
        _analyser->set_property("freeze-peak", vs.freeze.freeze.peak);
        _analyser->set_property("freeze-peak-en", vs.freeze.freeze.peak_en);
        _analyser->set_property("freeze-duration", vs.freeze.freeze.duration);
        _analyser->set_property("diff-cont", vs.freeze.diff.cont);
        _analyser->set_property("diff-cont-en", vs.freeze.diff.cont_en);
        _analyser->set_property("diff-peak", vs.freeze.diff.peak);
        _analyser->set_property("diff-peak-en", vs.freeze.diff.peak_en);
        _analyser->set_property("diff-duration", vs.freeze.diff.duration);
        _analyser->set_property("blocky-cont", vs.blocky.blocky.cont);
        _analyser->set_property("blocky-cont-en", vs.blocky.blocky.cont_en);
        _analyser->set_property("blocky-peak", vs.blocky.blocky.peak);
        _analyser->set_property("blocky-peak-en", vs.blocky.blocky.peak_en);
        _analyser->set_property("blocky-duration", vs.blocky.blocky.duration);
        _analyser->set_property("mark-blocks", vs.blocky.mark_blocks);
    }
}

static void
audiodata_callback (GstElement*,
                    guint64    ds,
                    GstBuffer* d,
                    guint64    es,
                    GstBuffer* e,
                    gpointer   _branch) {
    Glib::RefPtr<Gst::Buffer> data = Glib::wrap(d);
    Glib::RefPtr<Gst::Buffer> errors = Glib::wrap(e);
    Audio_branch* branch = (Audio_branch*)_branch;
    branch->parse_data_msg(ds, data, es, errors);
}

Audio_branch::Audio_branch(uint stream, uint chan, uint pid, std::shared_ptr<Audio_data> as) {
    _stream = stream;
    _channel = chan;
    _pid = pid;
    _audio_sender = as;

    _decoder->signal_pad_added().
        connect([this,stream,chan,pid](const Glib::RefPtr<Gst::Pad>& pad) {
                auto pcaps = pad->get_current_caps()->get_structure(0).get_name();
                vector<Glib::ustring> caps_toks = Glib::Regex::split_simple("/", pcaps);
                auto& type    = caps_toks[0];		    

                if (type != "audio") return;

                /* TODO add audio set */
                //pad->connect_property_changed("caps", [this, pad](){ set_video(pad); });

                auto conv      = Gst::ElementFactory::create_element("audioconvert");
                auto _analyser = Gst::ElementFactory::create_element("audioanalysis");
                if (! _analyser) Error_expn("Branch: audionalysis is not found");

                g_signal_connect(_analyser->gobj(), "data", G_CALLBACK(audiodata_callback), this);

                auto sink_pad = conv->get_static_pad("sink");
                auto src_pad  = _analyser->get_static_pad("src");
                
                _bin->add(conv)->add(_analyser);
                conv->link(_analyser);
                
                conv->sync_state_with_parent();
                _analyser->sync_state_with_parent();

                pad->link(sink_pad);

                auto p = shared_ptr<Pad>(new Pad(stream, chan, pid,
                                                 "audio", _bin, src_pad));
                _pads.push_back(p);
                _audio_pad_added.emit(p);
                _pad_added.emit(p->copy());
            });
}

void
Audio_branch::parse_data_msg(int64_t ds, Glib::RefPtr<Gst::Buffer> d,int64_t es, Glib::RefPtr<Gst::Buffer> e) {
    _audio_sender->parse_data_msg(_stream, _channel, _pid, ds,d,es,e);
}

void
Audio_branch::apply (const Settings& s) {
    const Settings::Audio& as = s.audio;

    if (_analyser) {
        _analyser->set_property("loss", as.loss);
        _analyser->set_property("silence-cont", as.silence.silence.cont);
        _analyser->set_property("silence-cont-en", as.silence.silence.cont_en);
        _analyser->set_property("silence-peak", as.silence.silence.peak);
        _analyser->set_property("silence-peak-en", as.silence.silence.peak_en);
        _analyser->set_property("silence-duration", as.silence.silence.duration);
        _analyser->set_property("loudness-cont", as.loudness.loudness.cont);
        _analyser->set_property("loudness-cont-en", as.loudness.loudness.cont_en);
        _analyser->set_property("loudness-peak", as.loudness.loudness.peak);
        _analyser->set_property("loudness-peak-en", as.loudness.loudness.peak_en);
        _analyser->set_property("loudness-duration", as.loudness.loudness.duration);
        _analyser->set_property("adv-diff", as.adv.adv_diff);
        _analyser->set_property("adv-buf", as.adv.adv_buf);
    }
}
