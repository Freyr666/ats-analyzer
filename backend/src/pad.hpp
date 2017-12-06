#ifndef PAD_H
#define PAD_H

#include <gstreamermm.h>
#include <gstreamermm/tee.h>
#include <string>
#include <vector>

#include "errexpn.hpp"

namespace Ats {

    class Pad {
    public:
	enum class Type { Video, Audio, Graph_volume, Unknown };
	
	Pad(uint, uint, uint, std::string, Glib::RefPtr<Gst::Bin>, Glib::RefPtr<Gst::Pad>);
	Pad(const Pad&&) = delete;
	Pad(Pad&) = delete;
        ~Pad();
        
        std::shared_ptr<Pad>   copy();
        
	uint                   stream()  { return _stream; }
	uint                   channel() { return _channel; }
	uint                   pid()     { return _pid; }
	Type                   type()    { return _t; }
	Glib::RefPtr<Gst::Pad> pad()     { return _pad; }
        sigc::signal<void>     signal_unlinked() { return _unlinked; }
        
    private:
        Pad() {}
        
	uint _stream;
	uint _channel;
	uint _pid;
	Type _t;
        Glib::RefPtr<Gst::Bin> _bin;
        Glib::RefPtr<Gst::Tee> _tee;
	Glib::RefPtr<Gst::Pad> _pad;
        sigc::signal<void>     _unlinked;
    };

}

#endif /* PAD_H */
