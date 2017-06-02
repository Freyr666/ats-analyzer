#ifndef SCHEMA_H
#define SCHEMA_H

#include <string>
#include "json.hpp"

using namespace std;
using json = nlohmann::json;

const json
compose_schema() {
/* ----------------------- Graph ----------------------------- */

    const json j_graph = {
        {"comment","JSON schema for Graph class"},
        {"type","object"},
        {"properties",{{"state",{{"type","string"},
                                 {"enum",{"null","pause","play","stop"}}}}}}
    };

/* ----------------------- Channel settings------------------- */


    const json j_error_overlay = {
        {"type","object"},
        {"properties",{{"enabled",{{"type","boolean"}}},
                       {"error_color",{{"$ref","#/definitions/color"}}}}}
    };

    const json j_channel_name = {
        {"type","object"},
        {"properties",{{"enabled",{{"type","boolean"}}},
                       {"font_size",{{"type","integer"}}},
                       {"fmt",{{"type","string"},
                               {"maxLength",200}}}}}
    };

    const json j_audio_meter = {
        {"type","object"},
        {"properties",{{"enabled",{{"type","boolean"}}},
                       {"position",{{"type","string"},
                                    {"enum",{"left","right"}}}}}}
    };

    const json j_status_bar = {
        {"type","object"},
        {"properties",{{"enabled",{{"type","boolean"}}},
                       {"position",{{"type","string"},
                                    {"enum",{"top_left","top_right",
                                             "left","right",
                                             "bottom_left","bottom_right"}}}},
                       {"aspect",{{"type","boolean"}}},
                       {"subtitles",{{"type","boolean"}}},
                       {"teletext",{{"type","boolean"}}},
                       {"eit",{{"type","boolean"}}},
                       {"qos",{{"type","boolean"}}},
                       {"scte35",{{"type","boolean"}}}}}
    };

    const json j_channel_settings = {
        {"type","object"},
        {"properties",{{"show_border",{{"type","boolean"}}},
                       {"border_color",{{"$ref","#/definitions/color"}}},
                       {"show_aspect_border",{{"type","boolean"}}},
                       {"aspect_border_color",{{"$ref","#/definitions/color"}}},
                       {"error_overlay",j_error_overlay},
                       {"channel_name",j_channel_name},
                       {"audio_meter",j_audio_meter},
                       {"status_bar",j_status_bar}}}
    };

/* ----------------------- Qoe settings----------------------- */

    const string qoe_param_regex = "^(black|luma|freeze|diff|blocky|loudness|silence)_(peak|cont)$";
    const string qoe_flag_regex = "^(black|luma|freeze|diff|blocky|loudness|silence)_(peak|cont)_en$";
    const string qoe_time_regex = "^(black|freeze|blocky|loudness|silence)_time$";

    const json j_qoe = {
        {"type","object"},
        {"properties",{{"adv_buf",{{"type","integer"},
                                   {"minimum",0},
                                   {"maximum",14400}}},
                       {"adv_diff",{{"type","number"},
                                    {"minimum",0},
                                    {"maximum",59}}},
                       {"vloss",{{"$ref","#/definitions/time"}}},
                       {"aloss",{{"$ref","#/definitions/time"}}}}},
        {"patternProperties",{{qoe_param_regex,{{"$ref","#/definitions/percent"}}},
                              {qoe_time_regex,{{"$ref","#/definitions/time"}}},
                              {qoe_flag_regex,{{"type","boolean"}}}}}
    };

/* ----------------------- Settings -------------------------- */

    const json j_settings = {
        {"comment","JSON schema for Settings class"},
        {"type","object"},
        {"properties",{{"qoe_settings",j_qoe},
                       {"channel_settings",j_channel_settings}}}
    };

/* ----------------------- Metadata -------------------------- */

    const json j_position = {
        {"type","object"},
        {"properties",{{"x",{{"$ref","#/definitions/coord"}}},
                       {"y",{{"$ref","#/definitions/coord"}}},
                       {"width",{{"$ref","#/definitions/coord"}}},
                       {"height",{{"$ref","#/definitions/coord"}}}}},
        {"required",{"x","y","width","height"}}
    };

    const json j_metapid = {
        {"type","object"},
        {"properties",{{"pid",{{"type","integer"},
                               {"minimum",0},
                               {"maximum",8191}}},
                       {"to_be_analyzed", {{"type", "boolean"}}},
                       {"position",j_position},
                       {"required",{"pid","to_be_analyzed","position"}}}}
    };

    const json j_metachannel = {
        {"type","object"},
        {"properties",{{"number",{{"type","integer"},
                                  {"minimum",0}}},
                       {"service_name",{{"type","string"}}},
                       {"provider_name",{{"type","string"}}},
                       {"pids",{{"type","array"},
                                {"uniqueItems",true},
                                {"items",j_metapid}}}}},
        {"required",{"number","pids"}}
    };

    const json j_metadata = {
        {"type","array"},
        {"uniqueItems",true},
        {"items",j_metachannel}
    };

/* ----------------------- Options --------------------------- */

    const json j_options = {
        {"comment","JSON schema for Options class"},
        {"type","object"},
        {"properties",{{"prog_list",j_metadata},
                       {"resolution",{{"type","object"},
                                      {"properties",{{"width",{{"type","integer"},
                                                               {"minimum",0},
                                                               {"maximum",1920}}},
                                                     {"height",{{"type","integer"},
                                                                {"minimum",0},
                                                                {"maximum",1080}}}}},
                                      {"required",{"width","height"}}}},
                       {"bg_color",{{"$ref","#/definitions/color"}}}}}
    };

/* ----------------------- Root ------------------------------ */

    const json j_definitions = {
        {"percent",{{"type","number"},
                    {"minimum",0},
                    {"maximum",100}}},
        {"time",{{"type","number"},
                 {"minimum",0},
                 {"maximum",3600}}},
        {"lufs",{{"type","number"},
                 {"minimum",-59},
                 {"maximum",-5}}},
        {"color",{{"type","integer"},
                  {"minimum",0},
                  {"maximum",16777215}}},
        {"coord",{{"type","integer"},
                  {"minimum",0}}}
    };

    const json j_root = {
        {"comment","JSON schema for data passed to backend"},
        {"type","object"},
        {"additionalProperties",false},
        {"properties",{{"options",j_options},
                       {"settings",j_settings},
                       {"graph",j_graph}}},
        {"definitions",j_definitions}
    };

    return j_root;
}

#endif /* SCHEMA_H */
