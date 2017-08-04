#ifndef SCHEMA_H
#define SCHEMA_H

#include <string>
#include "json.hpp"

#define LUMA_WHITE 235
#define LUMA_BLACK 16

using namespace std;
using json = nlohmann::json;

json merge (const json &a, const json &b) {
    json result = a.flatten();
    json tmp = b.flatten();
    for ( auto it = tmp.begin(); it != tmp.end(); ++it )
        result[it.key()] = it.value();
    return result.unflatten();
}

const json
compose_schema() {
/* ----------------------- Graph ----------------------------- */

    const json j_graph = {
        {"comment","JSON schema for Graph class"},
        {"type","object"},
        {"properties",{{"state",{{"type","string"},
                                 {"enum",{"null","pause","play","stop"}}}}}}
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
                       {"timebool _plugged = false;",{{"$ref","#/definitions/time"}}}}}
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
        {"properties",{{"qoe_settings",j_qoe}}}
    };

/* ----------------------- Metadata -------------------------- */
    const json j_metapid = {
        {"type","object"},
        {"properties",{{"pid",{{"$ref","#/definitions/pid"}}},
                       {"to_be_analyzed", {{"type", "boolean"}}},
                       {"required",{"pid","to_be_analyzed"}}}}
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
        {"properties",{{"prog_list",j_metadata}}}
    };

/* ---------------------- WM --------------------------------- */

    const json j_basic_window_or_widget = {
        {"type","object"},
        {"properties",{{"type",{{"type","string"}}},
                       {"position",{{"$ref","#/definitions/position"}}}}},
    };

    const json j_window_props = {
        {"type","object"},
        {"properties",{{"widgets",{{"type","array"},
                                   {"items",j_basic_window_or_widget},
                                   {"uniqueItems",true}}}}}
    };

    /* NOTE: should be extended to check various window and widgets types */

    const json j_window = merge(j_basic_window_or_widget, j_window_props);

    const json j_wm = {
        {"comment","JSON schema for WM class"},
        {"type","object"},
        {"properties",{{"background",{{"type","object"},
                                      /* FIXME add properties */
                                      {"properties",{{"color",{{"$ref","#/definitions/color"}}}}}}},
                       {"resolution",{{"type","array"},
                                      {"items",{{{"type","integer"},
                                                 {"minimum",0},
                                                 {"maximum",1920}},
                                                {{"type","integer"},
                                                 {"minimum",0},
                                                 {"maximum",1080}}}},
                                      {"additionalItems",false}}},
                       {"layout",{{"type","array"},
                                  {"items",{{"type","array"},
                                            {"items",{{{"type","string"}},
                                                      j_window}}}},
                                  {"uniqueItems",true}}}}}
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
                  {"minimum",0}}},
        {"position",{{"type","object"},
                     {"properties",{{"left",{{"$ref","#/definitions/coord"}}},
                                    {"top",{{"$ref","#/definitions/coord"}}},
                                    {"right",{{"$ref","#/definitions/coord"}}},
                                    {"bottom",{{"$ref","#/definitions/coord"}}}}},
                     {"required",{"left","top","right","bottom"}}}},
        {"pid",{{"type","integer"},
                {"minimum",0},
                {"maximum",8191}}}
    };

    const json j_root = {
        {"comment","JSON schema for data passed to backend"},
        {"type","object"},
        {"additionalProperties",false},
        {"properties",{{"options",j_options},
                       {"settings",j_settings},
                       {"graph",j_graph},
                       {"WM",j_wm}}},
        {"definitions",j_definitions}
    };

    return j_root;
}

#endif /* SCHEMA_H */
