#include "address.hpp"

using namespace Ats;

address
Ats::get_address(int stream,
	    const std::string a,
	    const int base) {
    return {.addr = a, .port = base+stream};
}
