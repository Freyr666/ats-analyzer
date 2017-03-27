#ifndef CHATTERER_H
#define CHATTERER_H

#include <glibmm.h>
#include <string>
#include "errexpn.hpp"

namespace Ats {

    inline std::string to_string (bool b) { return b ? "true" : "false"; }

    inline std::string add_indent (std::string& s, int indent = 2) {
        size_t start_pos = 0;
        const std::string from = "\n";
        const std::string to = from + std::string(indent, '\t');
        while(((start_pos = s.find(from, start_pos)) != std::string::npos) &&
              (start_pos != (s.length() - 1))) {
            s.replace(start_pos, from.length(), to);
            start_pos += to.length();
        }
        return s;
    }

    class Chatterer {
    public:
        struct Serializer_failure : public Error_expn {
            static constexpr const char* expn_overflow = "serializer buffer overflowed: ";
            Serializer_failure() : Error_expn() {}
            Serializer_failure(string s) : Error_expn(s) {}
        };
        struct Deserializer_failure : public Error_expn {
            static constexpr const char* expn_bool = " must be a boolean";
            static constexpr const char* expn_number = " must be a number";
            static constexpr const char* expn_string = " must be a string";
            static constexpr const char* expn_object = " must be an object";
            static constexpr const char* expn_array = " must be an array";
            static constexpr const char* expn_null = " must be null";
            
            Deserializer_failure() : Error_expn() {}
            Deserializer_failure(string s) : Error_expn(s) {}
        };
        
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
