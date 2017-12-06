#include "pad.hpp"

using namespace Ats;
using namespace std;

Pad::Pad(uint stream,
	 uint chan,
	 uint pid,
	 std::string type,
         Glib::RefPtr<Gst::Bin> b,
	 Glib::RefPtr<Gst::Pad> p) {

    if ( type == "video" ) _t = Pad::Type::Video;
    else if ( type == "audio" ) _t = Pad::Type::Audio;
    else _t = Pad::Type::Unknown;

    _stream = stream;
    _channel = chan;
    _pid = pid;
    _bin = b;
    _tee = Gst::Tee::create();

    _bin->add(_tee);
    _tee->sync_state_with_parent();

    auto tee_sink = _tee->get_static_pad("sink");
    p->link(tee_sink);
    auto mid_pad = _tee->request_pad(_tee->get_pad_template("src_%u"));

    auto ghost = Gst::GhostPad::create(mid_pad);
    ghost->set_active();
    _pad       = (Glib::RefPtr<Gst::Pad>) ghost;
    _bin->add_pad(_pad);

    //_pad->signal_unlinked().connect ([this](const Glib::RefPtr<Gst::Pad> p){ if (p) _unlinked.emit(); });
}

Pad::~Pad() {
    _bin->remove_pad(_pad);
    if (_pad->is_ghost_pad ()) {
        auto gp = (Glib::RefPtr<Gst::GhostPad>*)(&_pad);
        _tee->remove_pad((*gp)->get_target());
    } else {
        _tee->remove_pad(_pad);
    }
}

std::shared_ptr<Pad>
Pad::copy() {
    auto p = std::shared_ptr<Pad> (new Pad());

    p->_stream   = _stream;
    p->_channel  = _channel;
    p->_pid      = _pid;
    p->_t        = _t;
    p->_tee      = _tee;
    p->_bin      = _bin;
    auto mid_pad = _tee->request_pad(_tee->get_pad_template("src_%u"));
    auto ghost   = Gst::GhostPad::create(mid_pad);
    ghost->set_active();
    p->_pad      = (Glib::RefPtr<Gst::Pad>) ghost;
    p->_bin->add_pad(p->_pad);

    return p;
}
