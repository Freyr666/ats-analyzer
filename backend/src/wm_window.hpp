#ifndef WM_WINDOW_H
#define WM_WINDOW_H

#include <gstreamermm.h>

#include "wm_element.hpp"

namespace Ats {
    
    class Wm_window : public Wm_element {
    public:
	Wm_window ();
	Wm_window (const Wm_window&) = delete;
	Wm_window (Wm_window&&) = delete;
	virtual ~Wm_window () {}

	virtual void add_to_pipe (Glib::RefPtr<Gst::Bin>);
	virtual void plug(shared_ptr<Pad>); // plug source
	virtual void plug(Glib::RefPtr<Gst::Pad>); // plug sink
	virtual uint stream()  { if (_plugged) return _stream; else throw Not_plugged {}; }
	virtual uint channel() { if (_plugged) return _channel; else throw Not_plugged {}; }
	virtual uint pid()     { if (_plugged) return _pid; else throw Not_plugged {}; }
	virtual bool is_enabled();
	virtual void enable();
	virtual void disable();
        // set left upper and right lower corners' coords
	virtual void set_position(pair<int,int>,pair<int,int>);
	
    private:
	bool _plugged = false;
	uint _stream;
	uint _channel;
	uint _pid;
	bool _enabled = false;
    };

}
    
#endif /* WM_WINDOW_H */
