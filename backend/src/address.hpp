#ifndef ADDRESS_H
#define ADDRESS_H

#include <string>

namespace Ats {

  struct Address {
    std::string addr;
    int         port;
  };

  Address get_address(int, const std::string = "224.1.2.2", const int = 1234);

};

#endif /* ADDRESS_H */
