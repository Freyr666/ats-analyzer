#include "probe.hpp"
#include "address.hpp"
#include "parser.hpp"

#include <cstdio>
#include <iostream>

#include <gst/gst.h>
#include <gst/mpegts/mpegts.h>
#include <glib.h>

using namespace Ats;

Probe::Probe(int s) : m(s) {
    stream = s;

    address a = get_address(s);

    auto src   = Gst::ElementFactory::create_element("udpsrc");
    auto parse = Gst::ElementFactory::create_element("tsparse");
    auto sink  = Gst::ElementFactory::create_element("fakesink");

    pipe   = Gst::Pipeline::create();

    pipe->add(src)->add(parse)->add(sink);

    src->link(parse)->link(sink);

    src->set_property("address", a.addr);
    src->set_property("port", a.port);
    src->set_property("timeout", 5000000000);

    bus    = pipe->get_bus();

    bus->add_watch([this] (const Glib::RefPtr<Gst::Bus>& bus,
			   const Glib::RefPtr<Gst::Message>& msg) -> bool {
		       return this->on_bus_message(bus, msg);
		   });
}

Probe::Probe(Probe&& src) : m(src.m) {
    swap(this->pipe, src.pipe);
}

Probe::~Probe() {}

void
Probe::set_state(Gst::State s) {
    pipe->set_state(s);
}

bool
Probe::on_bus_message(const Glib::RefPtr<Gst::Bus>& bus,
		      const Glib::RefPtr<Gst::Message>& msg) {
    GstMpegtsSection*   section;
  
    switch (msg->get_message_type()) {
    case Gst::MESSAGE_ELEMENT: {
	if ((msg->get_source()->get_name().substr(0, 11) == "mpegtsparse") &&
	    (section = gst_message_parse_mpegts_section (msg->gobj()))) {

	    if(Parse::table(section, m))
		cerr << "Got table at " << stream << "\nData:\n" << m.to_string() << "\n";
	    
	    gst_mpegts_section_unref (section);
	} else {
	    cerr << msg->get_structure().get_name() << "\n";
	    if (msg->get_structure().get_name() == "GstUDPSrcTimeout") {
		m.clear();
		cerr << "Clear table at " << stream << "\nData:\n" << m.to_string() << "\n";
	    }
	}
    }
	break;
    default: break;
    }
    return    true;
}
