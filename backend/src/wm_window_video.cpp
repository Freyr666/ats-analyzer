#include "wm_window.hpp"

using namespace Ats;
using namespace std;

Wm_window_video::Wm_window_video() {
    _scale  = Gst::ElementFactory::create_element("videoscale");
    _caps   = Gst::ElementFactory::create_element("capsfilter");
    _caps->set_property("caps", Gst::Caps::create_from_string("video/x-raw,pixel-aspect-ratio=1/1"));
}

void
Wm_window_video::add_to_pipe (Glib::RefPtr<Gst::Bin> pipe) {
    pipe->add(_scale)->add(_caps);
    _scale->link(_caps);
}

void
Wm_window_video::plug(shared_ptr<Ats::Pad> src) {
    if (_plugged) return;
    _stream = src->stream();
    _channel = src->channel();
    _pid = src->pid();
    _plugged = true;
    src->pad()->link(_scale->get_static_pad("sink"));
}

void
Wm_window_video::plug(Glib::RefPtr<Gst::Pad> sink) {

}

std::string
Wm_window_video::name() {
    string rval = "Vid_";
    rval += to_string(_stream);
    rval += to_string(_pid);
    return rval;
}

bool
Wm_window_video::is_enabled() {

}

void
Wm_window_video::enable() {

}

void
Wm_window_video::disable() {

}

void
Wm_window_video::set_position(pair<int, int> luc, pair<int, int> rlc) {

}
