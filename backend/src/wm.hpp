#ifndef WM_H
#define WM_H

#include <vector>
#include <string>
#include <gstreamermm.h>

#include "chatterer.hpp"
#include "wm_config.hpp"
#include "wm_widget.hpp"
#include "wm_window.hpp"

namespace Ats {

    class Wm : public Chatterer, public Logger {
    public:
	Wm();
	Wm(Wm&&) = delete;
	Wm(const Wm&) = delete;

	void add_sink(uint stream, uint pid, Glib::RefPtr<Gst::Pad> sink);
	void add_to_bin(Glib::RefPtr<Gst::Bin>);
	Glib::RefPtr<Gst::Pad> get_src();

    private:
        Wm_config config;
	Glib::RefPtr<Gst::Element> background;
	Glib::RefPtr<Gst::Element> mixer;

	void update_config();
	void validate();
    };
    
}

#endif /* WM_H */
