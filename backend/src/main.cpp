#include "context.hpp"
#include "settings.hpp"

#include <iostream>

using namespace Ats;

int
main(int argc, char *argv[])
{
    Gst::init(argc, argv);

    try {
        Context c(Initial(argc, argv));
        c.run();
    } catch (Initial::Wrong_option& e) {
        if (e)
            std::cerr << "Error: " << e.what() << std::endl;
        std::cerr << Initial::usage(argv[0]) << std::endl;
    } catch (Context::Size_error) {
        std::cerr << "Error: at list one input uri should be provided" << std::endl;
        std::cerr << Initial::usage(argv[0]) << std::endl;
    }
    
    return 0;
}
