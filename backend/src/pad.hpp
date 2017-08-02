#ifndef PAD_H
#define PAD_H

#include <gstreamermm.h>
#include <string>
#include <vector>

#include "errexpn.hpp"

namespace Ats {

    class Pad {
    public:
	enum class Type { Video, Audio, Graph_volume, Unknown };
	
	Pad(uint, uint, uint, std::string, Glib::RefPtr<Gst::Pad>);
	Pad(const Pad&&) = delete;
	Pad(Pad&) = delete;
        ~Pad() { _destroyed.emit(); }

	uint                   stream()  { return _stream; }
	uint                   channel() { return _channel; }
	uint                   pid()     { return _pid; }
	Type                   type()    { return _t; }
	Glib::RefPtr<Gst::Pad> pad()     { return _pad; }
        sigc::signal<void>     signal_destroyed() { return _destroyed; }
        
    private:
	uint _stream;
	uint _channel;
	uint _pid;
	Type _t;
	Glib::RefPtr<Gst::Pad> _pad;
        sigc::signal<void>     _destroyed;
    };

}

#endif /* PAD_H */
