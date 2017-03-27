#ifndef VALIDATE_H
#define VALIDATE_H

#include <string>

namespace Ats {

    inline bool
    validate_uri (const std::string& s) {
	const char* prefix = "udp://";

	if (s.empty()) return false;
    
	const char* string = s.c_str();
	const char* symb = &string[0];

	// match prefix
	for (const char* psymb = prefix; *psymb != 0;){
	    if (*psymb != *symb) return false;
	    psymb++;
	    symb++;
	}
	// match addr
	const int field_size = 3;
	const int port_size = 4;
	int field = 4, field_num = field_size, port_num = port_size;
	// ip switch-case
    ip_switch:
	if (*symb >= '0' && *symb <= '9') {
	    if (field_num <= 0) return false;
	    symb++;
	    field_num--;
	    goto ip_switch;
	} else if (*symb == '.') {
	    if (field_num == field_size) return false;
	    symb++;
	    field--;
	    field_num = field_size;
	    goto ip_switch;
	} else if (*symb == ':') {
	    if (field != 1) return false;
	    symb++;
	    goto port_switch;
	} else if (*symb == 0) {
	    if (field != 1) return false;
	    goto valid;	
	} else {
	    return false;
	}
	// port switch-case
    port_switch:
	if (*symb >= '0' && *symb <= '9') {
	    if (port_num <= 0) return false;
	    symb++;
	    port_num--;
	    goto port_switch;
	} else if (*symb == 0) {
	    if (port_num < 0 || port_num == port_size) return false;
	    goto valid;
	} else {
	    return false;
	}
    valid:
	return true;
    }

}
    
#endif /* VALIDATE_H */
