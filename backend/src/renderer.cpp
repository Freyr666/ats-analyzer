#include "renderer.hpp"

using namespace Ats;

Video_renderer::Video_renderer() {
    _output = Gst::ElementFactory::create_element("glimagesink");
}

void
Video_renderer::add_to_pipe(const Glib::RefPtr<Gst::Bin> b) {
    if (_output) {
        b->add(_output);
        _output->sync_state_with_parent();
    } else {
        throw Error_expn("Video_renderer: could not add renderer to the graph");
    }
}

void
Video_renderer::plug(Wm & wm) {
    wm.plug(_output->get_static_pad("sink"));
}

Audio_renderer::Audio_renderer () {
    _output = Gst::ElementFactory::create_element("pulsesink");
}

void
Audio_renderer::add_to_pipe(const Glib::RefPtr<Gst::Bin> b) {
    if (_output) {
        b->add(_output);
        _output->sync_state_with_parent();
    } else {
        throw Error_expn("Audio_renderer: could not add renderer to the graph");
    }
}

void
Audio_renderer::plug (std::shared_ptr<Pad> p) {
    p->pad()->link(_output->get_static_pad("sink"));
}
