#include "wm.hpp"

using namespace std;
using namespace Ats;

void /* TODO err */
Wm::init(Glib::RefPtr<Gst::Bin> b) {
    bin        = b;
    mixer      = Gst::ElementFactory::create_element("glvideomixer");
    background = Gst::ElementFactory::create_element("videotestsrc");
    background->set_property("is_live", true);

    bin->add(mixer)->add(background);
    
    background_pad = mixer->get_request_pad("sink_%u");

    auto in_pad = background->get_static_pad("src");
    in_pad->link(background_pad);
}

void
Wm::add_sink(uint stream, uint pid, string type, Glib::RefPtr<Gst::Pad> sink) {
    /* create Window and Widgets */
}

void
Wm::on_remove_sink(uint stream, uint pid) {

}

Glib::RefPtr<Gst::Pad>
Wm::get_src() {
    return mixer->get_static_pad("src");
}

// TODO Chatterer

string
Wm::to_string() const {
    return "TODO";
}

json
Wm::serialize() const {
    return nullptr;
}

void
Wm::deserialize(const json&) {

}
