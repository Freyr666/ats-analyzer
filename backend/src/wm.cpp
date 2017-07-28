#include "wm.hpp"

using namespace std;
using namespace Ats;

void /* TODO err */
Wm::add_to_pipe(Glib::RefPtr<Gst::Bin> b) {
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
Wm::plug(shared_ptr<Pad> sink) {
    /* create Window and Widgets */
}

void
Wm::on_remove_sink(uint stream, uint pid) {

}
