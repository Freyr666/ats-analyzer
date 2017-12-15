#ifndef RENDERER_H
#define RENDERER_H

#include <gstreamermm.h>

#include "wm.hpp"
#include "pad.hpp"

namespace Ats {

    class Renderer {
    public:
	Renderer (int port) : _port(port) {}
	Renderer (const Renderer&&) = delete;
	Renderer (Renderer&) = delete;
	virtual ~Renderer() {}

	virtual void add_to_pipe (Glib::RefPtr<Gst::Bin>) = 0;

    private:
        int _port;
    };
    

    class Video_renderer : public Renderer {
    public:
	Video_renderer (int port);
	~Video_renderer () = default;
	void plug (Wm&);
        virtual void add_to_pipe (const Glib::RefPtr<Gst::Bin>);
    private:
        Glib::RefPtr<Gst::Element> _encoder;
        Glib::RefPtr<Gst::Element> _pay;
	Glib::RefPtr<Gst::Element> _output;
    };

    class Audio_renderer : public Renderer {
    public:
	Audio_renderer (int port);
	~Audio_renderer () = default;
	void plug (std::shared_ptr<Pad>);
	virtual void add_to_pipe (const Glib::RefPtr<Gst::Bin>);
    private:
        Glib::RefPtr<Gst::Element> _encoder;
        Glib::RefPtr<Gst::Element> _pay;
	Glib::RefPtr<Gst::Element> _output;
    };
}

#endif /* RENDERER_H */
