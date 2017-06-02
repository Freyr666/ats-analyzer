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
        Error_expn() noexcept {}
        Error_expn(string s) noexcept: _err(s) {}
        const char* what() const noexcept { return _err ? (*_err).c_str() : ""; }
        operator bool() const noexcept { return true; }
    };

}

#endif /* ERREXPN_H */
