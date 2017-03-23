#ifndef PROBE_H
#define PROBE_H

#include <string>
#include <gstreamermm.h>
#include <glibmm.h>
#include <exception>

#include "metadata.hpp"

using namespace Glib;

namespace Ats {

    class Probe {
    public:
	class No_pipe : std::exception {};
	
	int                   stream;
	string                uri;
	Metadata              metadata;

	sigc::signal<void,const Metadata&> updated;
	
	Probe(int stream, string uri);
	Probe(const Probe&) = delete;
	Probe(Probe&&) = delete;

	void        set_state(Gst::State);

    private:
	RefPtr<Gst::Pipeline> pipe;
	RefPtr<Gst::Bus>      bus;
	
	bool on_bus_message(const Glib::RefPtr<Gst::Bus>&,
			    const Glib::RefPtr<Gst::Message>&);
    };

};

#endif /* PROBE_H */
