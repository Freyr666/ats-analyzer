#include "branch.hpp"

using namespace std;
using namespace Ats;

unique_ptr<Branch>
Branch::create(std::string type) {

    if (type == "video") return unique_ptr<Branch>((Branch*) new Video_branch());
    else if (type == "audio") return unique_ptr<Branch>((Branch*) new Audio_branch());
    else return unique_ptr<Branch>(nullptr);
}

Branch::Branch() {
    auto _bin    = Gst::Bin::create();
    auto queue   = Gst::ElementFactory::create_element("queue");
    auto decoder = Gst::ElementFactory::create_element("decodebin");

    queue->set_property("max-size-buffers", 20000);
    queue->set_property("max-size-bytes", 12000000);
}

Video_branch::Video_branch(uint stream, uint chan, uint pid) {
    auto _bin     = Gst::Bin::create();
    auto queue   = Gst::ElementFactory::create_element("queue");
    auto decoder = Gst::ElementFactory::create_element("decodebin");

    queue->set_property("max-size-buffers", 20000);
    queue->set_property("max-size-bytes", 12000000);
}

Audio_branch::Audio_branch(uint stream, uint chan, uint pid) {
    
}
