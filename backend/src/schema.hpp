#ifndef SCHEMA_H
#define SCHEMA_H

#include <string>
#include "json.hpp"

#define LUMA_WHITE 235
#define LUMA_BLACK 16

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

    auto get_j_qoe_setting = [](string ref)-> const json {
        const json j_qoe_setting = {
            {"type","object"},
            {"properties",{{"peak_en",{{"type","boolean"}}},
                           {"peak",{{"$ref",ref}}},
                           {"cont_en",{{"type","boolean"}}},
                           {"cont",{{"$ref",ref}}}}}
        };
        return j_qoe_setting;
    };

    const json j_loss = {
        {"type","object"},
        {"properties",{{"vloss",{{"$ref","#/definitions/time"}}},
                       {"aloss",{{"$ref","#/definitions/time"}}}}}
    };

    const json j_black = {
        {"type","object"},
        {"properties",{{"black",get_j_qoe_setting("#/definitions/percent")},
                       {"luma",get_j_qoe_setting("#/definitions/luma")},
                       {"black_pixel",{{"type","integer"},
                                       {"minimum",LUMA_BLACK},
                                       {"maximum",LUMA_WHITE}}},
                       {"time",{{"$ref","#/definitions/time"}}}}}
    };

    const json j_freeze = {
        {"type","object"},
        {"properties",{{"freeze",get_j_qoe_setting("#/definitions/percent")},
                       {"diff",get_j_qoe_setting("#/definitions/diff")},
                       {"pixel_diff",{{"type","integer"},
                                      {"minimum",0},
                                      {"maximum",(LUMA_WHITE - LUMA_BLACK)}}},
                       {"time",{{"$ref","#/definitions/time"}}}}}
    };

    const json j_blocky = {
        {"type","object"},
        {"properties",{{"blocky",get_j_qoe_setting("#/definitions/percent")},
                       {"mark_blocks",{{"type","boolean"}}},
                       {"time",{{"$ref","#/definitions/time"}}}}}
    };

    const json j_silence = {
        {"type","object"},
        {"properties",{{"silence",get_j_qoe_setting("#/definitions/lufs")},
                       {"time",{{"$ref","#/definitions/time"}}}}}
    };

    const json j_loudness = {
        {"type","object"},
        {"properties",{{"loudness",get_j_qoe_setting("#/definitions/lufs")},
                       {"time",{{"$ref","#/definitions/time"}}}}}
    };

    const json j_adv = {
        {"type","object"},
        {"properties",{{"adv_diff",{{"type","number"},
                                    {"minimum",0},
                                    {"maximum",59}}}, // FIXME
                       {"adv_buf",{{"type","integer"},
                                   {"minimum",0},
                                   {"maximum",14400}}}}} // FIXME
    };

    const json j_qoe = {
        {"type","object"},
        {"properties",{{"loss",j_loss},
                       {"black",j_black},
                       {"freeze",j_freeze},
                       {"blocky",j_blocky},
                       {"silence",j_silence},
                       {"loudness",j_loudness},
                       {"adv",j_adv}}}
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

    const string port_regex = "([0-9]{1,4}|[1-5][0-9]{4}|6[0-4][0-9]{3}|65[0-4][0-9]{2}|655[0-2][0-9]|6553[0-5])";
    const string host_regex = R"((([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])\.){3}([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5]))";
    const string uri_regex = string(R"(^udp:\/\/)") + host_regex + "(:" + port_regex + ")?$";

    const json j_metastream = {
        {"type","object"},
        {"properties", {{"stream",{{"type","integer"},
                                   {"minimum", 0}}},
                        {"uri",{{"type","string"},
                                {"pattern",uri_regex}}},
                        {"channels",{{"type","array"},
                                     {"uniqueItems",true},
                                     {"items",j_metachannel}}}}},
        {"required",{"stream","channels"}}
    };

    const json j_metadata = {
        {"type","array"},
        {"uniqueItems",true},
        {"items",j_metastream}
    };

/* ----------------------- Options --------------------------- */

    const json j_options = {
        {"comment","JSON schema for Options class"},
        {"type","object"},
        {"properties",{{"prog_list",j_metadata},
                       {"mosaic_resolution",{{"type","array"},
                                             {"items",{{{"type","integer"},
                                                        {"minimum",0},
                                                        {"maximum",1920}},
                                                       {{"type","integer"},
                                                        {"minimum",0},
                                                        {"maximum",1080}}}},
                                             {"additionalItems",false}}},
                       {"mosaic_bg_color",{{"$ref","#/definitions/color"}}}}}
    };

/* ----------------------- Root ------------------------------ */

    const json j_definitions = {
        {"percent",{{"type","number"},
                    {"minimum",0},
                    {"maximum",100}}},
        {"luma",{{"type","number"},
                 {"minimum",LUMA_BLACK},
                 {"maximum",LUMA_WHITE}}},
        {"diff",{{"type","number"},
                 {"minimum",0},
                 {"maximum",(LUMA_WHITE - LUMA_BLACK)}}},
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
