#ifndef ERREXPN_H
#define ERREXPN_H

#include <string>
#include <exception>
#include <boost/optional.hpp>

using namespace std;

namespace Ats {

    class Error_expn : exception {
        boost::optional<string> _err;
    public:
        Error_expn() {}
        Error_expn(string s) : _err(s) {}
        string message() const { return _err ? *_err : ""; }
        operator bool() const { return true; }
    };

}

#endif /* ERREXPN_H */
