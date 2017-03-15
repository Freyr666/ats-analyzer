#include "address.hpp"

using namespace Ats;

Address
Ats::get_address(int stream,
	    const std::string a,
	    const int base) {
    return {.addr = a, .port = base+stream};
}
