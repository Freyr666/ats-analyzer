#ifndef ERREXPN_H
#define ERREXPN_H

#include <string>
#include <exception>

using namespace std;

namespace Ats {

    class Error_expn : exception {
	    string _err;
	public:
	    Error_expn(string s) : _err(s) {}
	    string message() const { return _err; }
	};

}

#endif /* ERREXPN_H */
