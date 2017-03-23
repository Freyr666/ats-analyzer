#include "context.hpp"
#include "settings.hpp"

using namespace Ats;

int
main(int argc, char *argv[])
{
    Gst::init(argc, argv);
    
    Context c(Initial(argc, argv));
    c.run();
    
    return 0;
}
