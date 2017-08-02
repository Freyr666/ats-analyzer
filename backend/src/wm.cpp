#include "wm.hpp"

using namespace std;
using namespace Ats;

void
Wm::reset() {
    _background_pad.reset();
    _background.reset();
    _mixer.reset();
    _windows.clear();
    _widgets.clear();
    _treeview.reset();
}

void /* TODO err */
Wm::add_to_pipe(const Glib::RefPtr<Gst::Bin> b) {
    _bin        = b;
    _mixer      = Gst::ElementFactory::create_element("glvideomixer");
    _background = Gst::ElementFactory::create_element("videotestsrc");
    _background->set_property("is_live", true);

    _bin->add(_mixer)->add(_background);
    
    _background_pad = _mixer->get_request_pad("sink_%u");

    auto in_pad = _background->get_static_pad("src");
    in_pad->link(_background_pad);
    apply_resolution();
}

void
Wm::plug(shared_ptr<Pad> src) {
    switch (src->type()) {
    case Pad::Type::Video: {							  
	auto w = shared_ptr<Wm_window> (new Wm_window_video ());
        auto name = w->gen_name();
	// TODO try catch
	w->add_to_pipe(_bin);
	w->plug(src);
	auto wres = _windows.try_emplace(name, w);
	if (wres.second) { // inserted
	    auto sink_pad = _mixer->get_request_pad("sink_%u");
	    w->plug(sink_pad);
	}
        w->signal_unlinked().connect([this, name](){ on_remove_window(name); });
	break;
    }
    case Pad::Type::Graph_volume:
    case Pad::Type::Audio:
    case Pad::Type::Unknown:
	break;
    }

    // GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN(_bin->gobj()), GST_DEBUG_GRAPH_SHOW_ALL, "pipeline");
}

void
Wm::plug (Glib::RefPtr<Gst::Pad> sink) {
    _mixer->get_static_pad("src")->link(sink);
}

void
Wm::on_remove_window(std::string name) {
    _treeview.remove_window(name);
    auto nh = _windows.extract(name); // window's destructor do the rest
}

void
Wm::set_resolution(const pair<uint,uint> r) {
    _resolution = r;
    apply_resolution();
}

void
Wm::apply_resolution() {
    _background_pad->set_property("height", _resolution.second);
    _background_pad->set_property("width", _resolution.first);
}
