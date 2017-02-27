#ifndef PROBE_H
#define PROBE_H

#include <string>
#include <gstreamermm.h>
#include <glibmm.h>

#include "metadata.hpp"

using namespace Glib;

namespace Ats {

    class Probe {
    public:
	int                   stream;
	RefPtr<Gst::Pipeline> pipe;
	RefPtr<Gst::Bus>      bus;
	Metadata              metadata;
	
	Probe(int stream);
	Probe(const Probe&) = delete;
	Probe(Probe&&);
	~Probe();

	void        set_state(Gst::State);

	sigc::signal<void,const Metadata&> updated;

    private:
	bool on_bus_message(const Glib::RefPtr<Gst::Bus>&,
			    const Glib::RefPtr<Gst::Message>&);
    };

};

#endif /* PROBE_H */
