#ifndef OPTIONS_H
#define OPTIONS_H

#include <vector>
#include <string>
#include <gstreamermm.h>

#include "chatterer.hpp"
#include "metadata.hpp"

using namespace std;
using namespace Glib;

namespace Ats {

    using json = nlohmann::json;

    class Graph;
    class Probe;

    class Options : public Chatterer, public Logger {

    public:

        /* ------- Json schema ------------------ */
        static constexpr const char* JSON_SCHEMA = R"({
            "comment": "JSON schema for Options class",
            "type": "object",
            "properties": {
                "prog_list": {
                    "type": "array",
                    "uniqueItems": true,
                    "items": {
                        "type": "object",
                        "properties": {
                            "number": {
                                "type": "integer",
                                "minimum": 0
                            },
                            "service_name": {"type": "string"},
                            "provider_name": {"type": "string"},
                            "pids": {
                                "type": "array",
                                "uniqueItems": true,
                                "items": {
                                    "type": "object",
                                    "properties": {
                                        "pid": {
                                            "type": "integer",
                                            "minimum": 0,
                                            "maximum": 8191
                                        },
                                        "to_be_analyzed": {"type": "boolean"},
                                        "position": {
                                            "type": "object",
                                            "properties": {
                                                "x": {"$ref": "#/definitions/coord"},
                                                "y": {"$ref": "#/definitions/coord"},
                                                "width": {"$ref": "#/definitions/coord"},
                                                "height": {"$ref": "#/definitions/coord"}
                                            },
                                            "required": ["x","y","width","height"]
                                        }
                                    },
                                    "required": ["pid", "to_be_analyzed","position"]
                                }
                            }
                        },
                        "required": ["number", "pids"]
                    }
                },
                "resolution": {
                    "type": "object",
                    "properties": {
                        "width": {
                            "type": "integer",
                            "minimum": 0,
                            "maximum": 1920
                        },
                        "height": {
                            "type": "integer",
                            "minimum": 0,
                            "maximum": 1080
                        },
                        "required": ["width", "height"]
                    }
                },
                "bg_color": {
                    "type": "integer",
                    "minimum": 0,
                    "maximum": 16777215
                }
            },
            "definitions": {
                "coord": {
                    "type": "integer",
                    "minimum": 0
                }
            }
        })";

        /* ------- Prog list -------------------- */
        vector<Metadata> data;

        /* ------- Mosaic settings -------------- */
        pair<uint,uint> resolution = make_pair(1920, 1080);
        uint bg_color = 0;

        sigc::signal<void,const Options&>   set;
        sigc::signal<void,const Options&>   destructive_set;
        sigc::signal<void>                  updated;
        
        Options(const std::string& n) : Chatterer(n) {}
        virtual ~Options() {}

        bool   is_empty () const;
        void   set_data(const Metadata&);
        void   set_pid(const uint, const uint, const uint, Meta_pid::Pid_type);
        Metadata*           find_stream (uint stream);
        const Metadata*     find_stream (uint stream) const;

        // Chatter implementation
        string to_string() const;
        json   serialize() const;
        void   deserialize(const json&);

        void operator=(const Metadata& m) { set_data(m); }

        void   connect(Probe&);
        void   connect(Graph&);
    };
};

#endif /* OPTIONS_H */
