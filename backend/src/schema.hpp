#ifndef SCHEMA_H
#define SCHEMA_H

#include <string>

using namespace std;

string
compose_schema() {
/* ----------------------- Graph ----------------------------- */

    const string j_graph = R"({
        "comment":"JSON schema for Graph class",
        "type":"object",
        "properties":{
            "state":{"type":"string",
                     "enum":["null","pause","play","stop"]}
        }
    })";

/* ----------------------- Channel settings------------------- */


    const string j_error_overlay = R"({
        "type":"object",
        "properties":{
            "enabled":{"type":"boolean"},
            "error_color":{"$ref":"#/definitions/color"}
        }
    })";

    const string j_channel_name = R"({
        "type":"object",
        "properties":{
            "enabled":{"type":"boolean"},
            "font_size":{"type":"integer"},
            "fmt":{
                "type":"string",
                "maxLength":200}
        }
    })";

    const string j_audio_meter = R"({
        "type":"object",
        "properties":{
            "enabled":{"type":"boolean"},
            "position":{"type":"string",
                        "enum":["left","right"]}
        }
    })";

    const string j_status_bar = R"({
        "type":"object",
        "properties":{
            "enabled":{"type":"boolean"},
            "position":{"type":"string",
                        "enum":["top_left","top_right",
                                "left","right",
                                "bottom_left","bottom_right"]},
            "aspect":{"type":"boolean"},
            "subtitles":{"type":"boolean"},
            "teletext":{"type":"boolean"},
            "eit":{"type":"boolean"},
            "qos":{"type":"boolean"},
            "scte35":{"type":"boolean"}
        }
    })";

    const string j_channel_settings = string(R"({
        "type":"object",
        "properties":{
            "show_border":{"type":"boolean"},
            "border_color":{"$ref":"#/definitions/color"},
            "show_aspect_border":{"type":"boolean"},
            "aspect_border_color":{"$ref":"#/definitions/color"},
            "error_overlay":)") + j_error_overlay + R"(,
            "channel_name":)" + j_channel_name + R"(,
            "audio_meter":)" + j_audio_meter + R"(,
            "status_bar":)" + j_status_bar + R"(
        }
    })";

/* ----------------------- Qoe settings----------------------- */

    const string qoe_param_regex = "^(black|luma|freeze|diff|blocky|loudness|silence)_(peak|cont)$";
    const string qoe_flag_regex = "^(black|luma|freeze|diff|blocky|loudness|silence)_(peak|cont)_en$";
    const string qoe_time_regex = "^(black|freeze|blocky|loudness|silence)_time$";

    const string j_qoe = string(R"({
        "type":"object",
        "properties":{
            "adv_buf":{
                "type":"integer",
                "minimum":0,
                "maximum":14400
            },
            "adv_diff":{
                "type":"number",
                "minimum":0,
                "maximum":59
            },
            "vloss":{"$ref":"#/definitions/time"},
            "aloss":{"$ref":"#/definitions/time"}
        },
        "patternProperties":{
            )") + "\"" + qoe_param_regex + "\"" + R"(:{"$ref":"#/definitions/percent"},
            )" + "\"" + qoe_time_regex + "\"" + R"(:{"$ref":"#/definitions/time"},
            )" + "\"" + qoe_flag_regex + "\"" + R"(:{"type":"boolean"}
        }
    })";
    
/* ----------------------- Settings -------------------------- */

    const string j_settings = string(R"({
        "comment":"JSON schema for Settings class",
        "type":"object",
        "properties":{
            "qoe_settings":)") + j_qoe + R"(,
            "channel_settings":)" + j_channel_settings + R"(
        }
    })";

/* ----------------------- Metadata -------------------------- */

    const string j_position = R"({
        "type":"object",
        "properties":{
            "x":{"$ref":"#/definitions/coord"},
            "y":{"$ref":"#/definitions/coord"},
            "width":{"$ref":"#/definitions/coord"},
            "height":{"$ref":"#/definitions/coord"}
        },
        "required":["x","y","width","height"]
    })";

    const string j_metapid = string(R"({
        "type":"object",
        "properties":{
            "pid":{
                "type":"integer",
                "minimum":0,
                "maximum":8191
            },
            "to_be_analyzed": {"type": "boolean"},
            "position":)") + j_position + R"(,
            "required":["pid","to_be_analyzed","position"]
        }
    })";

    const string j_metachannel = string(R"({
        "type":"object",
        "properties":{
            "number":{
                "type":"integer",
                "minimum":0
            },
            "service_name":{"type":"string"},
            "provider_name":{"type":"string"},
            "pids":{
                "type":"array",
                "uniqueItems":true,
                "items":)") + j_metapid + R"(
            }
        },
        "required":["number","pids"]
    })";

    const string j_metadata = string(R"({
        "type":"array",
        "uniqueItems":true,
        "items":)") + j_metachannel + R"(
    })";


/* ----------------------- Options --------------------------- */

    const string j_options = string(R"({
        "comment":"JSON schema for Options class",
        "type":"object",
        "properties":{
            "prog_list":)") + j_metadata + R"(,
            "resolution":{
                "type":"object",
                "properties":{
                    "width":{
                        "type":"integer",
                        "minimum":0,
                        "maximum":1920
                    },
                    "height":{
                        "type":"integer",
                        "minimum":0,
                        "maximum":1080
                    },
                    "required":["width","height"]
                }
            },
            "bg_color":{"$ref":"#/definitions/color"}
        }
    })";

/* ----------------------- Root ------------------------------ */

    const string j_definitions = R"({
        "percent":{
            "type":"number",
            "minimum":0,
            "maximum":100
        },
        "time":{
            "type":"number",
            "minimum":0,
            "maximum":3600
        },
        "lufs":{
            "type":"number",
            "minimum":-59,
            "maximum":-5
        },
        "color":{
            "type":"integer",
            "minimum":0,
            "maximum":16777215
        },
        "coord":{
            "type":"integer",
            "minimum":0
        }
    })";

    const string j_root = string(R"({
        "comment":"JSON schema for data passed to backend",
        "type":"object",
        "additionalProperties":false,
        "properties":{
            "options":)") + j_options + R"(,
            "settings":)" + j_settings + R"(,
            "graph":)" + j_graph + R"(
        },
        "definitions":)" + j_definitions + R"(
    })";

    return j_root;
}

#endif /* SCHEMA_H */
