#include "context.hpp"

using namespace Ats;

int
main(int argc, char *argv[])
{
    Gst::init(argc, argv);

    Context c;
    c.run();
    
    return 0;
}
