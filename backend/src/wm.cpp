#include "wm.hpp"

using namespace std;
using namespace Ats;

void /* TODO err */
Wm::add_to_pipe(Glib::RefPtr<Gst::Bin> b) {
    _bin        = b;
    _mixer      = Gst::ElementFactory::create_element("glvideomixer");
    _background = Gst::ElementFactory::create_element("videotestsrc");
    _background->set_property("is_live", true);

    _bin->add(_mixer)->add(_background);
    
    _background_pad = _mixer->get_request_pad("sink_%u");

    auto in_pad = _background->get_static_pad("src");
    in_pad->link(_background_pad);
}

void
Wm::plug(shared_ptr<Pad> sink) {
    /* create Window and Widgets */
}

void
Wm::on_remove_sink(uint stream, uint pid) {

}
