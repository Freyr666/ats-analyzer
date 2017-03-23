#ifndef CHATTERER_H
#define CHATTERER_H

#include <glibmm.h>
#include <string>

namespace Ats {

    inline std::string to_string (bool b) { return b ? "true" : "false"; }

    inline void add_indent (std::string& s, int indent = 2) {
        for (auto it = s.begin(); it != s.end(); ++it) {
            if ( *it == '\n' && it < (s.end() - 1)) {
                s.insert(it+1, indent, '\t');
                it += indent;
            }
        }
    }

    class Chatterer {
    public:
        class Serializer_failure : std::exception {};

        sigc::signal<void,const Chatterer&> send;
        sigc::signal<void,const std::string&> send_err;
        sigc::signal<void,const std::string&> send_log;

        void talk ()                     { send.emit(*this); }
        void error(const std::string& s) { send_err.emit(s); }
        void log  (const std::string& s) { send_log.emit(s); }
	
        virtual std::string to_string() const = 0;	
        virtual std::string to_json()   const = 0;
        virtual std::string to_msgpack()   const = 0;

        virtual void   of_json(const std::string&) = 0;
        virtual void   of_msgpack(const std::string&) = 0;

        static std::string err_to_json(const std::string& s) {
            std::string rval = "{\"error\":\"";
            rval += s;
            return rval + "\"}";
        }
        static std::string err_to_msgpack(const std::string& s) {
            return s;
        }
    };

}

#endif /* CHATTERER_H */
