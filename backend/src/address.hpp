#ifndef ADDRESS_H
#define ADDRESS_H

#include <string>

namespace Ats {

    struct address {
	std::string addr;
	int         port;
    };

    address get_address(int, const std::string = "224.1.2.2", const int = 1234);
    
};

#endif /* ADDRESS_H */
