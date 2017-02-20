#include <cstdio>
#include <gstreamermm.h>
#include <glibmm.h>

#include "probe.hpp"

using namespace Ats;

int
main(int argc, char *argv[])
{
    Gst::init(argc, argv);

    auto main_loop = Glib::MainLoop::create();
    
    auto t0 = Probe(0);

    auto t1 = Probe(1);

    t0.set_state(Gst::STATE_PLAYING);

    t1.set_state(Gst::STATE_PLAYING);
    
    main_loop->run();
    
    return 0;
}
