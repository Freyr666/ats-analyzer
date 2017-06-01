#ifndef CHATTERER_H
#define CHATTERER_H

#include <glibmm.h>
#include <string>
#include "json.hpp"
#include "json-schema.hpp"
#include "errexpn.hpp"

/* sets the value of *type* and *name* (if present),
   taken from the root of json object,
   to variable *obj.name* */
#define set_value_from_json(j_obj,obj,name,type) do{  \
        if (j.find(#name) != j.end())           \
            (obj).name = j_obj.at(#name).get<type>(); \
    } while (0)

#define set_json

namespace Ats {

    using json = nlohmann::json;
	
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

    class Logger {
    public:
        sigc::signal<void,const std::string&> send_log;
        void log  (const std::string& s) { send_log.emit(s); }
    };

    class Chatterer {
    public:
        struct Serializer_failure : public Error_expn {
            Serializer_failure() : Error_expn() {}
            Serializer_failure(string s) : Error_expn(s) {}
        };
	
        struct Deserializer_failure : public Error_expn {
            Deserializer_failure() : Error_expn() {}
            Deserializer_failure(string s) : Error_expn(s) {}
        };

        struct Validator_failure : public Error_expn {
            static constexpr const char* schema_failure = "JSON Schema loading failure: ";
            static constexpr const char* json_failure = "Invalid JSON: ";

            Validator_failure() : Error_expn() {}
            Validator_failure(string s) : Error_expn(s) {}
        };

        Chatterer(const std::string n) : name(n) {}
	
        Chatterer(const Chatterer&) = delete;
        Chatterer(Chatterer&&) = delete;
	
        const std::string name;
        
        sigc::signal<void,const Chatterer&>   send;
        sigc::signal<void,const std::string&> send_err;

        void talk ()                     { send.emit(*this); }
        void error(const std::string& s) { send_err.emit(s); }

        

        virtual std::string to_string()    const = 0;
        virtual json        serialize()    const = 0;
        virtual void        deserialize(const json&) = 0;

        static void validate(const json& j, const json& j_schema) {

            using nlohmann::json;
            using nlohmann::json_uri;
            using nlohmann::json_schema_draft4::json_validator;

            json_validator validator;

            try {
                validator.set_root_schema(j_schema);
            } catch (const std::exception &e) {
                throw Validator_failure((std::string)Validator_failure::schema_failure + e.what());
            }

            try {
                validator.validate(j);
            } catch (const std::exception &e) {
                throw Validator_failure((std::string)Validator_failure::json_failure + e.what());
            }
        };

        static std::string err_to_json(const std::string& s) {
            std::string rval = "{\"error\":\"";
            rval += s;
            return rval + "\"}";
        }
        static std::string err_to_msgpack(const std::string& s) {
            return s;
        }
    };

    class Chatterer_proxy {
    public:
        Chatterer_proxy() {}
        Chatterer_proxy(const Chatterer_proxy&) = delete;
        Chatterer_proxy(Chatterer_proxy&&) = delete;
	
        sigc::signal<void,const std::string&>      send;
        sigc::signal<void,const std::string&>      send_err;
	
        virtual void forward_talk(const Chatterer&) = 0;
        virtual void forward_error(const std::string&) = 0;
        virtual void dispatch(const std::string&) = 0;

        void connect(Chatterer& c) {
            c.send.connect(sigc::mem_fun(this, &Chatterer_proxy::forward_talk));
            c.send_err.connect(sigc::mem_fun(this, &Chatterer_proxy::forward_error));
        }
	
    };

}

#endif /* CHATTERER_H */
