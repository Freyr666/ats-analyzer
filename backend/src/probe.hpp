#ifndef PROBE_H
#define PROBE_H

#include <string>
#include <gstreamermm.h>
#include <glibmm.h>

using namespace Glib;

namespace Ats {

    class Probe {
    public:
	int                   stream;
	RefPtr<Gst::Pipeline> pipe;
	RefPtr<Gst::Bus>      bus;

	Probe(int stream);
	Probe(const Probe&) = delete;
	Probe(Probe&&);
	// ~Probe();

	void        set_state(Gst::State);
	std::string to_string();

    private:
	static bool on_bus_message(const Glib::RefPtr<Gst::Bus>&,
				   const Glib::RefPtr<Gst::Message>&,
				   int);
    };

};

#endif /* PROBE_H */
