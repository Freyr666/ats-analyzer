#include "renderer.hpp"

using namespace Ats;

Video_renderer::Video_renderer() {
    _output = Gst::ElementFactory::create_element("glimagesink");
}

void
Video_renderer::add_to_pipe(const Glib::RefPtr<Gst::Bin> b) {
    b->add(_output);
    _output->sync_state_with_parent();
}

void
Video_renderer::plug(Wm & wm) {
    wm.plug(_output->get_static_pad("sink"));
}

Audio_renderer::Audio_renderer () {

}

void
Audio_renderer::add_to_pipe(const Glib::RefPtr<Gst::Bin> b) {

}

void
Audio_renderer::plug (std::shared_ptr<Pad>) {

}
