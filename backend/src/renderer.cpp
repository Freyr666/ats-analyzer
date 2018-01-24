#include "renderer.hpp"

using namespace Ats;

Video_renderer::Video_renderer(int port) : Renderer(port) {
    _encoder = Gst::ElementFactory::create_element("vaapivp8enc");
    _pay     = Gst::ElementFactory::create_element("rtpvp8pay");
    _output  = Gst::ElementFactory::create_element("udpsink");
    //_encoder->set_property("bitrate", 102400);
    //_encoder->set_property("yac-qi", 40);
    //_encoder->set_property("sharpness-level", 3);
    //_encoder->set_property("loop-filter-level", 63);
    _encoder->set_property("rate-control", 2);
    //_encoder->set_property("keyframe-period", 15);
    // _encoder->set_property("target-bitrate", 105000000);
    //_encoder->set_property("min-quantizer", 50);
    //_encoder->set_property("max-quantizer", 63);
    //_encoder->set_property("cq-level", 54);
    //_encoder->set_property("buffer-size", 600000000);
    //_encoder->set_property("cpu-used", 16);
    // _encoder->set_property("threads", 8);
    //_encoder->set_property("lag-in-frames", 4);
    _output->set_property("host", std::string("127.0.0.1"));
    _output->set_property("port", port);
    _output->set_property("async", false);
    _output->set_property("sync", false);
}

void
Video_renderer::add_to_pipe(const Glib::RefPtr<Gst::Bin> b) {
    if (_output && _pay && _encoder) {
        b->add(_encoder)->add(_pay)->add(_output);
        _encoder->link(_pay)->link(_output);
        _encoder->sync_state_with_parent();
        _pay->sync_state_with_parent();
        _output->sync_state_with_parent();
    } else {
        throw Error_expn("Video_renderer: could not add renderer to the graph");
    }
}

void
Video_renderer::plug(Wm & wm) {
    wm.plug(_encoder->get_static_pad("sink"));
}

Audio_renderer::Audio_renderer (int port)  : Renderer(port) {
    _encoder = Gst::ElementFactory::create_element("opusenc");
    _pay     = Gst::ElementFactory::create_element("rtpopuspay");
    _output = Gst::ElementFactory::create_element("udpsink");
    _output->set_property("host", std::string("127.0.0.1"));
    _output->set_property("port", port);
}

void
Audio_renderer::add_to_pipe(const Glib::RefPtr<Gst::Bin> b) {
    if (_output && _pay && _encoder) {
        b->add(_encoder)->add(_pay)->add(_output);
        _encoder->link(_pay)->link(_output);
        _encoder->sync_state_with_parent();
        _pay->sync_state_with_parent();
        _output->sync_state_with_parent();
    } else {
        throw Error_expn("Audio_renderer: could not add renderer to the graph");
    }
}

void
Audio_renderer::plug (std::shared_ptr<Pad> p) {
    p->pad()->link(_encoder->get_static_pad("sink"));
}
