#include "wm.hpp"

using namespace std;
using namespace Ats;

void
Wm::reset() {
    _background_pad.reset();
    _background.reset();
    _mixer.reset();
    _widgets.clear();
    _treeview.reset();
    _bin.reset();
}

void /* TODO err */
Wm::add_to_pipe(const Glib::RefPtr<Gst::Bin> b) {
    _bin        = b;
    _mixer      = Gst::ElementFactory::create_element("compositor");
    _background = Gst::ElementFactory::create_element("videotestsrc");
    _mixer->set_property("background", 1);
    _background->set_property("is_live", true);
    _background->set_property("pattern", 2);

    _bin->add(_mixer)->add(_background);
    
    _background_pad = _mixer->get_request_pad("sink_%u");
    _background_pad->set_property("zorder", 1);

    auto in_pad = _background->get_static_pad("src");
    in_pad->link(_background_pad);
    apply_resolution();
}

void
Wm::plug(shared_ptr<Pad> src) {
    switch (src->type()) {
    case Pad::Type::Video: {							  
        auto w = shared_ptr<Wm_widget> (new Wm_widget_video ());
        // TODO try catch
        w->add_to_pipe(_bin);
        w->plug(src);
        auto uid = w->gen_uid();
        auto wres = _widgets.try_emplace(uid, w);

        if (wres.second) { // inserted
            auto sink_pad = _mixer->get_request_pad("sink_%u");
            w->plug(sink_pad);
        }
        w->signal_unlinked().connect([this, uid](){ on_remove_widget(uid); });
        w->signal_linked().connect([this](){ talk(); });
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
Wm::on_remove_widget(std::string uid) {
    _treeview.remove_widget(uid);
    auto nh = _widgets.extract(uid); // window's destructor do the rest
}

void
Wm::set_resolution(const resolution_t r) {
    _resolution = r;
    apply_resolution();
}

void
Wm::apply_resolution() {
    _background_pad->set_property("height", _resolution.second);
    _background_pad->set_property("width", _resolution.first);
}
