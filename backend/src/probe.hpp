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
	Metadata              metadata;

	sigc::signal<void,const Metadata&> updated;
	
	Probe(int stream);
	Probe(const Probe&) = delete;
	Probe(Probe&&);
	~Probe();

	void        set_state(Gst::State);

    private:
	RefPtr<Gst::Pipeline> pipe;
	RefPtr<Gst::Bus>      bus;
	
	bool on_bus_message(const Glib::RefPtr<Gst::Bus>&,
			    const Glib::RefPtr<Gst::Message>&);
    };

};

#endif /* PROBE_H */
