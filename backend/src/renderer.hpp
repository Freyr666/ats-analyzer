#ifndef RENDERER_H
#define RENDERER_H

#include <gstreamermm.h>

#include "wm.hpp"
#include "pad.hpp"

namespace Ats {

    class Renderer {
    public:
	Renderer () {}
	Renderer (const Renderer&&) = delete;
	Renderer (Renderer&) = delete;
	virtual ~Renderer() {}

	virtual void add_to_pipe (Glib::RefPtr<Gst::Bin>) = 0;
    };
    

    class Video_renderer : public Renderer {
    public:
	Video_renderer ();
	~Video_renderer () {}
	void plug (Wm&);
        virtual void add_to_pipe (const Glib::RefPtr<Gst::Bin>);
    private:
	Glib::RefPtr<Gst::Element> _output;
    };

    class Audio_renderer : public Renderer {
    public:
	Audio_renderer ();
	~Audio_renderer () {}
	void plug (std::shared_ptr<Pad>);
	virtual void add_to_pipe (const Glib::RefPtr<Gst::Bin>);
    };
}

#endif /* RENDERER_H */
