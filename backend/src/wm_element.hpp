#ifndef WM_ELEMENT_H
#define WM_ELEMENT_H

#include <gstreamermm.h>
#include "pad.hpp"

using namespace std;

namespace Ats {

    class Wm_element {
    public:
	class Not_plugged : public std::exception {};

	virtual void add_to_pipe (Glib::RefPtr<Gst::Bin>) = 0;
	virtual void plug(shared_ptr<Pad>) = 0; // plug source
	virtual void plug(Glib::RefPtr<Gst::Pad>) = 0; // plug sink
	virtual uint stream() = 0;
	virtual uint channel() = 0;
	virtual uint pid() = 0;
	virtual bool is_enabled() = 0;
	virtual void enable() = 0;
	virtual void disable() = 0;
        // set left upper and right lower corners' coords
	virtual void set_position(pair<int,int>,pair<int,int>) = 0;
    };
    
}

#endif /* WM_ELEMENT_H */
