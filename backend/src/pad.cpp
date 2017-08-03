#include "pad.hpp"

using namespace Ats;
using namespace std;

Pad::Pad(uint stream,
	 uint chan,
	 uint pid,
	 std::string type,
	 Glib::RefPtr<Gst::Pad> p) {

    if ( type == "video" ) _t = Pad::Type::Video;
    else if ( type == "audio" ) _t = Pad::Type::Audio;
    else _t = Pad::Type::Unknown;

    _pad = p;
    _stream = stream;
    _channel = chan;
    _pid = pid;

    //_pad->signal_unlinked().connect ([this](const Glib::RefPtr<Gst::Pad> p){ if (p) _unlinked.emit(); });
}
