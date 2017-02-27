#include <cstdio>
#include <gstreamermm.h>
#include <glibmm.h>

#include "probe.hpp"
#include "options.hpp"

using namespace Ats;

int
main(int argc, char *argv[])
{
    Gst::init(argc, argv);

    auto main_loop = Glib::MainLoop::create();

    auto opts = Options();
    
    auto t0 = Probe(0);
    auto t1 = Probe(1);
    auto t2 = Probe(2);

    opts.connect(t0);
    opts.connect(t1);
    opts.connect(t2);
    
    t0.set_state(Gst::STATE_PLAYING);

    t1.set_state(Gst::STATE_PLAYING);

    t2.set_state(Gst::STATE_PLAYING);
    
    main_loop->run();
    
    return 0;
}
