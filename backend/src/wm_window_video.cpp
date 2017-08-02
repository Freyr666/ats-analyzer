#include "wm_window.hpp"

using namespace Ats;
using namespace std;

Wm_window_video::Wm_window_video() : _position (make_pair(0,0), make_pair(0,0)) {
    _scale  = Gst::ElementFactory::create_element("videoscale");
    _caps   = Gst::ElementFactory::create_element("capsfilter");
    _caps->set_property("caps", Gst::Caps::create_from_string("video/x-raw,pixel-aspect-ratio=1/1"));
}

Wm_window_video::~Wm_window_video () {
    //_scale->unparent();
    //_caps->unparent();
}

void
Wm_window_video::add_to_pipe (const Glib::RefPtr<Gst::Bin> pipe) {
    pipe->add(_scale)->add(_caps);
    _scale->link(_caps);
    _scale->sync_state_with_parent();
    _caps->sync_state_with_parent();
}

void
Wm_window_video::plug(shared_ptr<Ats::Pad> src) {
    if (_plugged) return;
    _stream = src->stream();
    _channel = src->channel();
    _pid = src->pid();
    _plugged = true;
    src->pad()->link(_scale->get_static_pad("sink"));
    src->signal_unlinked().connect([this](){ _unlinked.emit(); });
}

void
Wm_window_video::plug(Glib::RefPtr<Gst::Pad> sink) {
    _caps->get_static_pad("src")->link(sink);
    _mixer_pad = sink;
}

std::string
Wm_window_video::gen_name() {
    string rval = "Vid_";
    rval += to_string(_stream);
    rval += to_string(_pid);
    return rval;
}

bool
Wm_window_video::is_enabled() {
    return _enabled;
}

void
Wm_window_video::enable() {
    
}

void
Wm_window_video::disable() {

}

void
Wm_window_video::set_position(const Wm_position& pos) {
    _position = pos;
    apply_position ();
}

Wm_position
Wm_window_video::get_position() {
    return _position;
}

void
Wm_window_video::apply_position () {
    if (_mixer_pad && _position) {
        _mixer_pad->set_property("height", _position.get_height());
        _mixer_pad->set_property("width", _position.get_width());
        _mixer_pad->set_property("xpos", _position.get_x());
        _mixer_pad->set_property("ypos", _position.get_y());
    }
}
