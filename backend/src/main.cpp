#include <cstdio>
#include <gstreamermm.h>

int
main(int argc, char *argv[])
{
    Gst::init(argc, argv);
    printf("Hello, World!\n");
    return 0;
}
