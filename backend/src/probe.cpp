#include "probe.hpp"
#include "address.hpp"

#include <cstdio>

using namespace Ats;

Probe::Probe(int s) {
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

    bus    = pipe->get_bus();

    bus->add_watch([s] (const Glib::RefPtr<Gst::Bus>& bus,
			const Glib::RefPtr<Gst::Message>& msg) -> bool {
		       return on_bus_message(bus, msg, s);
		   });
}

Probe::Probe(Probe&& src) {
    swap(this->pipe, src.pipe);
}

void
Probe::set_state(Gst::State s) {
    pipe->set_state(s);
}

std::string
Probe::to_string() {
    return "TODO";
}

bool
Probe::on_bus_message(const Glib::RefPtr<Gst::Bus>& bus,
		      const Glib::RefPtr<Gst::Message>& msg,
		      int val) {
    fprintf(stderr, "Got msg from %d\n", val);
    return true;
}
