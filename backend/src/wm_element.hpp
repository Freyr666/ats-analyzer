#ifndef WM_ELEMENT_H
#define WM_ELEMENT_H

#include <gstreamermm.h>
#include "pad.hpp"
#include "wm_position.hpp"

using namespace std;

namespace Ats {

    class Wm_element {
    public:
	class Not_plugged : public std::exception {};

	virtual void add_to_pipe (const Glib::RefPtr<Gst::Bin>) = 0;
	virtual void plug(shared_ptr<Pad>) = 0; // plug source
	virtual void plug(Glib::RefPtr<Gst::Pad>) = 0; // plug sink
 	virtual std::string gen_name() = 0;
	virtual bool is_enabled() = 0;
	virtual void enable() = 0;
	virtual void disable() = 0;
	virtual void set_position(const Wm_position&) = 0;
        virtual Wm_position get_position() = 0;
        sigc::signal <void > signal_unlinked() { return _unlinked; }
        
    protected:
        sigc::signal <void > _unlinked;
    };
    
}

#endif /* WM_ELEMENT_H */
