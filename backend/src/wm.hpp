#ifndef WM_H
#define WM_H

#include <vector>
#include <string>
#include <gstreamermm.h>
#include <map>

#include "chatterer.hpp"
#include "wm_widget.hpp"
#include "wm_window.hpp"
#include "position.hpp"
#include "metadata.hpp"
#include "pad.hpp"

namespace Ats {

    class Wm : public Chatterer, public Logger {
    public:
	Wm() : Chatterer ("WM") {}
	Wm(Wm&&) = delete;
	Wm(const Wm&) = delete;
	virtual ~Wm() {}

	void add_to_pipe (Glib::RefPtr<Gst::Bin>);
	void plug (std::shared_ptr<Pad>);

	Glib::RefPtr<Gst::Pad> get_src_pad() { return mixer->get_static_pad("src"); }

	// Chatterer
        string to_string() const;
        json   serialize() const;
        void   deserialize(const json&);

    private:
	pair<uint,uint> resolution = make_pair(1920, 1080);
	std::map<std::pair<uint,uint>,Wm_window> windows;
	std::map<std::pair<uint,uint>,Wm_widget> widgets;
	Glib::RefPtr<Gst::Bin>     bin;
	Glib::RefPtr<Gst::Element> background;
	Glib::RefPtr<Gst::Pad>     background_pad;
	Glib::RefPtr<Gst::Element> mixer;

	void on_remove_sink(const uint stream, const uint pid);

	void update_config();
	void validate();

	void   set_resolution(const pair<uint,uint>);
	void   set_position(uint, uint, const Position&);
    };
    
}

#endif /* WM_H */
