#ifndef WM_ELEMENT_H
#define WM_ELEMENT_H

#include <gstreamermm.h>

using namespace std;

namespace Ats {

    class Wm_element {
    public:
	Wm_element(const Wm_element&) = delete;
	Wm_element(Wm_element&&) = delete;

	virtual uint stream();
	virtual uint channel();
	virtual uint pid();
	virtual bool is_enabled();
	virtual void enable();
	virtual void disable();
        // set left upper and right lower corners' coords
	virtual void set_position(pair<int,int>,pair<int,int>);

	virtual Glib::RefPtr<Gst::Pad> get_src();
	virtual Glib::RefPtr<Gst::Pad> get_sink();
    };
    
}

#endif /* WM_ELEMENT_H */
