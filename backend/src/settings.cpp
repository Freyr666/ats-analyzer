#include "settings.hpp"
#include "json.hpp"
#include "validate.hpp"

#include <cstdio>
#include <algorithm>
#include <iostream>

using namespace std;
using namespace Ats;

Initial::Initial(int argc, char** argv) {
    /* Options:
     * -h help
     * -m msgtype
     * -o output uri
     */
    if (argc < 2) throw Wrong_option("Too few arguments");
    
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] != '-') {
            string uri = argv[i];
            if (! validate_uri(uri)) throw Wrong_option("Bad uri: " + uri);
            uris.push_back(uri);
        } else {
            switch (argv[i][1]) {
            case 'h': {
                throw Wrong_option();
                break;
            }
            case 'o': {
                if (i == (argc - 1)) throw Wrong_option("-o requires argument");
                string uri = argv[++i];
                if (! validate_uri(uri)) throw Wrong_option("Bad output uri: " + uri);
                multicast_address = uri;
                break;
            }
            case 'm': {
                if (i == (argc - 1)) throw Wrong_option("-m requires argument");
                string msgt = argv[++i];
                if (msgt == "debug") {
                    msg_type = Msg_type::Debug;
                } else if (msgt == "json") {
                    msg_type = Msg_type::Json;
                } else if (msgt == "msgpack") {
                    msg_type = Msg_type::Msgpack;
                } else {
                    throw Wrong_option("Bad message type: " + msgt);
                }
                break;
            }
            default:
                throw Wrong_option("Unknown option: " + string(argv[i]));
                break;
            }
        }
    }
}

string
Initial::usage (string prog_name) {
    string rval = "Usage:\n";
    rval += prog_name;
    rval += " [-opt arg] uri1 [uri2 uri3]\n"
        "Options:\n"
        "\t-o\toutput uri\n"
        "\t-m\tipc message type [json | msgpack | debug]\n"
        "\t-h\thelp\n"
        "Additional:\n"
        "\turi format: udp://[ip] or udp://[ip]:[port]\n";
    return rval;
}

/* ---------- Channel settings ----------- */

string
Settings::Channel_settings::Error_overlay::to_string() const {
    string rval = "Enabled: ";
    rval += Ats::to_string(enabled);
    rval += ", Error color: ";
    rval += std::to_string(error_color);
    rval += "\n";
    return rval;
}

void
Ats::to_json(json& j, const Settings::Channel_settings::Error_overlay& eo) {
    j = {{"enabled", eo.enabled},
         {"error_color", eo.error_color}};
}

void
Ats::from_json(const json& j, Settings::Channel_settings::Error_overlay& eo) {
}

string
Settings::Channel_settings::Channel_name::to_string() const {
    string rval = "Enabled: ";
    rval += Ats::to_string(enabled);
    rval += ", Font size: ";
    rval += std::to_string(font_size);
    rval += ", Format: ";
    rval += fmt;
    rval += "\n";
    return rval;
}

void
Ats::to_json(json& j, const Settings::Channel_settings::Channel_name& cn) {
    j = {{"enabled", cn.enabled},
         {"font_size", cn.font_size},
         {"fmt", cn.fmt}};
}

void
Ats::from_json(const json& j, Settings::Channel_settings::Channel_name& cn) {
}


string
Settings::Channel_settings::Audio_meter::to_string() const {
    string rval = "Enabled: ";
    rval += Ats::to_string(enabled);
    rval += ", Position: ";
    rval += (position == Audio_meter_pos::Left ? "left" : "right");
    rval += "\n";
    return rval;
}

void
Ats::to_json(json& j, const Settings::Channel_settings::Audio_meter& am) {
    using Audio_meter_pos = Settings::Channel_settings::Audio_meter::Audio_meter_pos;
    j = {{"enabled", am.enabled},
         {"position", (am.position == Audio_meter_pos::Left ? "left" :
                       am.position == Audio_meter_pos::Right ? "right" :
                       "right")}};
}

void
Ats::from_json(const json& j, Settings::Channel_settings::Audio_meter& am) {
}

string
Settings::Channel_settings::Status_bar::to_string() const {
    string rval = "Enabled: ";
    rval += Ats::to_string(enabled);
    rval += ", Position: ";
    rval += (position == Status_bar_pos::Top_left ? "top_left" :
             position == Status_bar_pos::Top_right ? "top_right" :
             position == Status_bar_pos::Left ? "left" :
             position == Status_bar_pos::Right ? "right" :
             position == Status_bar_pos::Bottom_left ? "bottom_left" :
             position == Status_bar_pos::Bottom_right ? "bottom_right" :
             "top_left");
    rval += ", Aspect: ";
    rval += Ats::to_string(aspect);
    rval += ", Subtitles: ";
    rval += Ats::to_string(subtitles);
    rval += ", Teletext: ";
    rval += Ats::to_string(teletext);
    rval += ", Eit: ";
    rval += Ats::to_string(eit);
    rval += ", Qos: ";
    rval += Ats::to_string(qos);
    rval += ", Scte35: ";
    rval += Ats::to_string(scte35);
    rval += "\n";
    return rval;
}

void
Ats::to_json(json& j, const Settings::Channel_settings::Status_bar& sb) {
    using Status_bar_pos = Settings::Channel_settings::Status_bar::Status_bar_pos;
    j = {{"enabled", sb.enabled},
         {"position", (sb.position == Status_bar_pos::Top_left ? "top_left" :
                       sb.position == Status_bar_pos::Top_right ? "top_right" :
                       sb.position == Status_bar_pos::Left ? "left" :
                       sb.position == Status_bar_pos::Right ? "right" :
                       sb.position == Status_bar_pos::Bottom_left ? "bottom_left" :
                       sb.position == Status_bar_pos::Bottom_right ? "bottom_right" :
                       "top_left")},
         {"aspect", sb.aspect},
         {"subtitles", sb.subtitles},
         {"teletext", sb.teletext},
         {"eit", sb.eit},
         {"qos", sb.qos},
         {"scte35", sb.scte35}};
}

void
Ats::from_json(const json& j, Settings::Channel_settings::Status_bar& sb) {
}

string
Settings::Channel_settings::to_string() const {
    string rval = "Show border:\n\t";
    rval += Ats::to_string(show_border);
    rval += "\nBorder color:\n\t";
    rval += std::to_string(border_color);
    rval += "\nShow aspect border:\n\t";
    rval += Ats::to_string(show_aspect_border);
    rval += "\nAspect border color:\n\t";
    rval += std::to_string(aspect_border_color);
    rval += "\nError overlay:\n\t";
    rval += error_overlay.to_string();
    rval += "Channel name:\n\t";
    rval += channel_name.to_string();
    rval += "Audio meter:\n\t";
    rval += audio_meter.to_string();
    rval += "Status bar:\n\t";
    rval += status_bar.to_string();
    return rval;
}

void
Ats::to_json(json& j, const Settings::Channel_settings& c) {
    j = {{"show_border", c.show_border},
         {"border_color", c.border_color},
         {"show_aspect_border", c.show_aspect_border},
         {"aspect_border_color", c.aspect_border_color},
         {"error_overlay", c.error_overlay},
         {"channel_name", c.channel_name},
         {"audio_meter", c.audio_meter},
         {"status_bar", c.status_bar}};
}

void
Ats::from_json(const json& j, Settings::Channel_settings& sb) {
}

/* ---------- QoE settings --------------- */

string
Settings::Qoe_settings::to_string() const {
    string rval = "Vloss:\n\t";
    rval += std::to_string(vloss);
    rval += "\nAloss:\n\t";
    rval += std::to_string(aloss);
    rval += "\nBlack cont en:\n\t";
    rval += Ats::to_string(black_cont_en);
    rval += "\nBlack cont:\n\t";
    rval += std::to_string(black_cont);
    rval += "\nBlack peak en:\n\t";
    rval += Ats::to_string(black_peak_en);
    rval += "\nBlack peak:\n\t";
    rval += std::to_string(black_peak);
    rval += "\nLuma cont en:\n\t";
    rval += Ats::to_string(luma_cont_en);
    rval += "\nLuma cont:\n\t";
    rval += std::to_string(luma_cont);
    rval += "\nLuma peak en:\n\t";
    rval += Ats::to_string(luma_peak_en);
    rval += "\nLuma peak:\n\t";
    rval += std::to_string(luma_peak);
    rval += "\nBlack time:\n\t";
    rval += std::to_string(black_time);
    rval += "\nBlack pixel:\n\t";
    rval += std::to_string(black_pixel);
    rval += "\nFreeze cont en:\n\t";
    rval += Ats::to_string(freeze_cont_en);
    rval += "\nFreeze cont:\n\t";
    rval += std::to_string(freeze_cont);
    rval += "\nFreeze peak en:\n\t";
    rval += Ats::to_string(freeze_peak_en);
    rval += "\nFreeze peak:\n\t";
    rval += std::to_string(freeze_peak);
    rval += "\nDiff cont en:\n\t";
    rval += Ats::to_string(diff_cont_en);
    rval += "\nDiff cont:\n\t";
    rval += std::to_string(diff_cont);
    rval += "\nDiff peak en:\n\t";
    rval += Ats::to_string(diff_peak_en);
    rval += "\nDiff peak:\n\t";
    rval += std::to_string(diff_peak);
    rval += "\nFreeze time:\n\t";
    rval += std::to_string(freeze_time);
    rval += "\nPixel diff:\n\t";
    rval += std::to_string(pixel_diff);
    rval += "\nBlocky cont en:\n\t";
    rval += Ats::to_string(blocky_cont_en);
    rval += "\nBlocky cont:\n\t";
    rval += std::to_string(blocky_cont);
    rval += "\nBlocky peak en:\n\t";
    rval += Ats::to_string(blocky_peak_en);
    rval += "\nBlocky peak:\n\t";
    rval += std::to_string(blocky_peak);
    rval += "\nBlocky time:\n\t";
    rval += std::to_string(blocky_time);
    rval += "\nMark blocks:\n\t";
    rval += Ats::to_string(mark_blocks);
    rval += "\nSilence cont en:\n\t";
    rval += Ats::to_string(silence_cont_en);
    rval += "\nSilence cont:\n\t";
    rval += std::to_string(silence_cont);
    rval += "\nSilence peak en:\n\t";
    rval += Ats::to_string(silence_peak_en);
    rval += "\nSilence peak:\n\t";
    rval += std::to_string(silence_peak);
    rval += "\nSilence time:\n\t";
    rval += std::to_string(silence_time);
    rval += "\nLoudness cont en:\n\t";
    rval += Ats::to_string(loudness_cont_en);
    rval += "\nLoudness cont:\n\t";
    rval += std::to_string(loudness_cont);
    rval += "\nLoudness peak en:\n\t";
    rval += Ats::to_string(loudness_peak_en);
    rval += "\nLoudness peak:\n\t";
    rval += std::to_string(loudness_peak);
    rval += "\nLoudness time:\n\t";
    rval += std::to_string(loudness_time);
    rval += "\nAdv diff:\n\t";
    rval += std::to_string(adv_diff);
    rval += "\nAdv buf:\n\t";
    rval += std::to_string(adv_buf);
    rval += "\n";
    return rval;
}

void
Ats::to_json(json& j, const Settings::Qoe_settings& qoe) {
    j = {{"vloss", qoe.vloss},
         {"aloss", qoe.aloss},
         {"black_cont_en", qoe.black_cont_en},
         {"black_cont", qoe.black_cont},
         {"black_peak_en", qoe.black_peak_en},
         {"black_peak", qoe.black_peak},
         {"luma_cont_en", qoe.luma_cont_en},
         {"luma_cont", qoe.luma_cont},
         {"luma_peak_en", qoe.luma_peak},
         {"luma_peak", qoe.luma_peak},
         {"black_time", qoe.black_time},
         {"black_pixel", qoe.black_pixel},
         {"freeze_cont_en", qoe.freeze_cont_en},
         {"freeze_cont", qoe.freeze_cont},
         {"freeze_peak_en", qoe.freeze_peak_en},
         {"freeze_peak", qoe.freeze_peak},
         {"diff_cont_en", qoe.diff_cont_en},
         {"diff_cont", qoe.diff_cont},
         {"diff_peak_en", qoe.diff_peak_en},
         {"diff_peak", qoe.luma_peak},
         {"freeze_time", qoe.freeze_time},
         {"pixel_diff", qoe.pixel_diff},
         {"blocky_cont_en", qoe.blocky_cont_en},
         {"blocky_cont", qoe.blocky_cont},
         {"blocky_peak_en", qoe.blocky_peak_en},
         {"blocky_peak", qoe.blocky_peak},
         {"blocky_time", qoe.blocky_time},
         {"mark_blocks", qoe.mark_blocks},
         {"silence_cont_en", qoe.silence_cont_en},
         {"silence_cont", qoe.silence_cont},
         {"silence_peak_en", qoe.silence_peak_en},
         {"silence_peak", qoe.silence_peak},
         {"silence_time", qoe.silence_time},
         {"loudness_cont_en", qoe.loudness_cont_en},
         {"loudness_cont", qoe.loudness_cont},
         {"loudness_peak_en", qoe.loudness_peak_en},
         {"loudness_peak", qoe.loudness_peak},
         {"loudness_time", qoe.loudness_time},
         {"adv_diff", qoe.adv_diff},
         {"adv_buf", qoe.adv_buf}};
}

void
Ats::from_json(const json& j, Settings::Qoe_settings& qoe) {
}

/* ---------- Settings -------------------- */

void
Settings::init(Initial& i) {
    if(i.multicast_address) {
	return;
    }
}

// Chatter implementation

string
Settings::to_string() const {
    string rval = "Settings:\n\tQoe settings:\n\t\t";
    string qoe_string = qoe_settings.to_string();
    rval += Ats::add_indent(qoe_string);
    rval += "\tChannel_settings:\n\t\t";
    string channel_string = channel_settings.to_string();
    rval += Ats::add_indent(channel_string);
    rval += "\n";
    return rval;
}

json
Settings::serialize() const {
    json j = json{{"qoe_settings", qoe_settings},
                  {"channel_settings", channel_settings}};
    return j;
}

void
Settings::deserialize(const json& j) {
    
}

string
Settings::to_json() const {
    return "";
}

void
Settings::of_json(json& js) {
    using Df = Chatterer::Deserializer_failure;
    
    constexpr const char* div = "::";
    bool o_set = false;

    if (!js.is_object()) throw Df(string("Settings") + Df::expn_object);

    for (json::iterator el = js.begin(); el != js.end(); ++el) {
        const string jk = el.key();
        auto jv = el.value();
        
        if (jk == "qoe_settings") {
            if (!jv.is_object()) throw Df(jk + Df::expn_object);
            
            for (json::iterator it = jv.begin(); it != jv.end(); ++it) {
                const std::string k = it.key();
                auto v = it.value();

                if (k == "vloss") {
                    if (!v.is_number()) throw Df(jk + div + k + Df::expn_number);
                    qoe_settings.vloss = v.get<float>();
                    o_set = true;
                }
                else if (k == "aloss") {
                    if (!v.is_number()) throw Df(jk + div + k + Df::expn_number);
                    qoe_settings.aloss = v.get<float>();
                    o_set = true;
                }
                else if (k == "black_cont_en") {
                    if (!v.is_boolean()) throw Df(jk + div + k + Df::expn_bool);
                    qoe_settings.black_cont_en = v.get<bool>();
                    o_set = true;
                }
                else if (k == "black_cont") {
                    if (!v.is_number()) throw Df(jk + div + k + Df::expn_number);
                    qoe_settings.black_cont = v.get<float>();
                    o_set = true;
                }
                else if (k == "black_peak_en") {
                    if (!v.is_boolean()) throw Df(jk + div + k + Df::expn_bool);
                    qoe_settings.black_peak_en = v.get<bool>();
                    o_set = true;
                }
                else if (k == "black_peak") {
                    if (!v.is_number()) throw Df(jk + div + k + Df::expn_number);
                    qoe_settings.black_peak = v.get<float>();
                    o_set = true;
                }
                else if (k == "luma_cont_en") {
                    if (!v.is_boolean()) throw Df(jk + div + k + Df::expn_bool);
                    qoe_settings.luma_cont_en = v.get<bool>();
                    o_set = true;
                }
                else if (k == "luma_cont") {
                    if (!v.is_number()) throw Df(jk + div + k + Df::expn_number);
                    qoe_settings.luma_cont = v.get<float>();
                    o_set = true;
                }
                else if (k == "luma_peak_en") {
                    if (!v.is_boolean()) throw Df(jk + div + k + Df::expn_bool);
                    qoe_settings.luma_peak_en = v.get<bool>();
                    o_set = true;
                }
                else if (k == "luma_peak") {
                    if (!v.is_number()) throw Df(jk + div + k + Df::expn_number);
                    qoe_settings.luma_peak = v.get<float>();
                    o_set = true;
                }
                else if (k == "black_time") {
                    if (!v.is_number()) throw Df(jk + div + k + Df::expn_number);
                    qoe_settings.black_time = v.get<float>();
                    o_set = true;
                }
                else if (k == "black_pixel") {
                    if (!v.is_number()) throw Df(jk + div + k + Df::expn_number);
                    qoe_settings.black_pixel = v.get<uint>();
                    o_set = true;
                }
                else if (k == "freeze_cont_en") {
                    if (!v.is_boolean()) throw Df(jk + div + k + Df::expn_bool);
                    qoe_settings.freeze_cont_en = v.get<bool>();
                    o_set = true;
                }
                else if (k == "freeze_cont") {
                    if (!v.is_number()) throw Df(jk + div + k + Df::expn_number);
                    qoe_settings.freeze_cont = v.get<float>();
                    o_set = true;
                }
                else if (k == "freeze_peak_en") {
                    if (!v.is_boolean()) throw Df(jk + div + k + Df::expn_bool);
                    qoe_settings.freeze_peak_en = v.get<bool>();
                    o_set = true;
                }
                else if (k == "freeze_peak") {
                    if (!v.is_number()) throw Df(jk + div + k + Df::expn_number);
                    qoe_settings.freeze_peak = v.get<float>();
                    o_set = true;
                }
                else if (k == "diff_cont_en") {
                    if (!v.is_boolean()) throw Df(jk + div + k + Df::expn_bool);
                    qoe_settings.diff_cont_en = v.get<bool>();
                    o_set = true;
                }
                else if (k == "diff_cont") {
                    if (!v.is_number()) throw Df(jk + div + k + Df::expn_number);
                    qoe_settings.diff_cont = v.get<float>();
                    o_set = true;
                }
                else if (k == "diff_peak_en") {
                    if (!v.is_boolean()) throw Df(jk + div + k + Df::expn_bool);
                    qoe_settings.diff_peak_en = v.get<bool>();
                    o_set = true;
                }
                else if (k == "diff_peak") {
                    if (!v.is_number()) throw Df(jk + div + k + Df::expn_number);
                    qoe_settings.diff_peak = v.get<float>();
                    o_set = true;
                }
                else if (k == "freeze_time") {
                    if (!v.is_number()) throw Df(jk + div + k + Df::expn_number);
                    qoe_settings.freeze_time = v.get<float>();
                    o_set = true;
                }
                else if (k == "pixel_diff") {
                    if (!v.is_number()) throw Df(jk + div + k + Df::expn_number);
                    qoe_settings.pixel_diff = v.get<uint>();
                    o_set = true;
                }
                else if (k == "blocky_cont_en") {
                    if (!v.is_boolean()) throw Df(jk + div + k + Df::expn_bool);
                    qoe_settings.blocky_cont_en = v.get<bool>();
                    o_set = true;
                }
                else if (k == "blocky_cont") {
                    if (!v.is_number()) throw Df(jk + div + k + Df::expn_number);
                    qoe_settings.blocky_cont = v.get<float>();
                    o_set = true;
                }
                else if (k == "blocky_peak_en") {
                    if (!v.is_boolean()) throw Df(jk + div + k + Df::expn_bool);
                    qoe_settings.blocky_peak_en = v.get<bool>();
                    o_set = true;
                }
                else if (k == "blocky_peak") {
                    if (!v.is_number()) throw Df(jk + div + k + Df::expn_number);
                    qoe_settings.blocky_peak = v.get<float>();
                    o_set = true;
                }
                else if (k == "blocky_time") {
                    if (!v.is_number()) throw Df(jk + div + k + Df::expn_number);
                    qoe_settings.blocky_time = v.get<float>();
                    o_set = true;
                }
                else if (k == "mark_blocks") {
                    if (!v.is_boolean()) throw Df(jk + div + k + Df::expn_bool);
                    qoe_settings.mark_blocks = v.get<bool>();
                    o_set = true;
                }
                else if (k == "silence_cont_en") {
                    if (!v.is_boolean()) throw Df(jk + div + k + Df::expn_bool);
                    qoe_settings.silence_cont_en = v.get<bool>();
                    o_set = true;
                }
                else if (k == "silence_cont") {
                    if (!v.is_number()) throw Df(jk + div + k + Df::expn_number);
                    qoe_settings.silence_cont = v.get<float>();
                    o_set = true;
                }
                else if (k == "silence_peak_en") {
                    if (!v.is_boolean()) throw Df(jk + div + k + Df::expn_bool);
                    qoe_settings.silence_peak_en = v.get<bool>();
                    o_set = true;
                }
                else if (k == "silence_peak") {
                    if (!v.is_number()) throw Df(jk + div + k + Df::expn_number);
                    qoe_settings.silence_peak = v.get<float>();
                    o_set = true;
                }
                else if (k == "silence_time") {
                    if (!v.is_number()) throw Df(jk + div + k + Df::expn_number);
                    qoe_settings.silence_time = v.get<float>();
                    o_set = true;
                }
                else if (k == "loudness_cont_en") {
                    if (!v.is_boolean()) throw Df(jk + div + k + Df::expn_bool);
                    qoe_settings.loudness_cont_en = v.get<bool>();
                    o_set = true;
                }
                else if (k == "loudness_cont") {
                    if (!v.is_number()) throw Df(jk + div + k + Df::expn_number);
                    qoe_settings.loudness_cont = v.get<float>();
                    o_set = true;
                }
                else if (k == "loudness_peak_en") {
                    if (!v.is_boolean()) throw Df(jk + div + k + Df::expn_bool);
                    qoe_settings.loudness_peak_en = v.get<bool>();
                    o_set = true;
                }
                else if (k == "loudness_peak") {
                    if (!v.is_number()) throw Df(jk + div + k + Df::expn_number);
                    qoe_settings.loudness_peak = v.get<float>();
                    o_set = true;
                }
                else if (k == "loudness_time") {
                    if (!v.is_number()) throw Df(jk + div + k + Df::expn_number);
                    qoe_settings.loudness_time = v.get<float>();
                    o_set = true;
                }
                else if (k == "adv_diff") {
                    if (!v.is_number()) throw Df(jk + div + k + Df::expn_number);
                    qoe_settings.adv_diff = v.get<float>();
                    o_set = true;
                }
                else if (k == "adv_buf") {
                    if (!v.is_number()) throw Df(jk + div + k + Df::expn_number);
                    qoe_settings.adv_buf = v.get<uint>();
                    o_set = true;
                }
            }
        }
        else if (jk == "channel_settings") {
            if (!jv.is_object()) throw Df(jk + Df::expn_object);
            for (json::iterator it = jv.begin(); it != jv.end(); ++it) {
                const std::string k = it.key();
                auto v = it.value();

                if (k == "error_overlay") {
                    if (!v.is_object()) throw Df(jk + div + k + Df::expn_object);
                    for (json::iterator eo_it = v.begin(); eo_it != v.end(); ++eo_it) {
                        const std::string eo_k = eo_it.key();
                        auto eo_v = eo_it.value();

                        if (eo_k == "enabled") {
                            if (!eo_v.is_boolean())
                                throw Df(jk + div + k + div + eo_k + Df::expn_bool);
                            channel_settings.error_overlay.enabled = eo_v.get<bool>();
                            o_set = true;
                        }
                        else if (eo_k == "error_color") {
                            if (!eo_v.is_number())
                                throw Df(jk + div + k + div + eo_k + Df::expn_number);
                            channel_settings.error_overlay.error_color = eo_v.get<uint>();
                            o_set = true;
                        }
                    }
                }
                else if (k == "channel_name") {
                    if (!v.is_object()) throw Df(jk + div + k + Df::expn_object);
                    for (json::iterator cn_it = v.begin(); cn_it != v.end(); ++cn_it) {
                        const std::string cn_k = cn_it.key();
                        auto cn_v = cn_it.value();

                        if (cn_k == "enabled") {
                            if (!cn_v.is_boolean())
                                throw Df(jk + div + k + div + cn_k + Df::expn_bool);
                            channel_settings.channel_name.enabled = cn_v.get<bool>();
                            o_set = true;
                        }
                        else if (cn_k == "font_size") {
                            if (!cn_v.is_number())
                                throw Df(jk + div + k + div + cn_k + Df::expn_number);
                            channel_settings.channel_name.font_size = cn_v.get<int>();
                            o_set = true;
                        }
                        else if (cn_k == "fmt") {
                            if (!cn_v.is_string())
                                throw Df(jk + div + k + div + cn_k + Df::expn_string);
                            channel_settings.channel_name.fmt = cn_v.get<std::string>();
                            o_set = true;
                        }
                    }
                }
                else if (k == "audio_meter") {
                    if (!v.is_object()) throw Df(jk + div + k + Df::expn_object);
                    for (json::iterator am_it = v.begin(); am_it != v.end(); ++am_it) {
                        const std::string am_k = am_it.key();
                        auto am_v = am_it.value();

                        if (am_k == "enabled") {
                            if (!am_v.is_boolean())
                                throw Df(jk + div + k + div + am_k + Df::expn_bool);
                            channel_settings.audio_meter.enabled = am_v.get<bool>();
                            o_set = true;
                        }
                        else if (am_k == "position") {
                            if (!am_v.is_string())
                                throw Df(jk + div + k + div + am_k + Df::expn_string);
                            std::string pos = am_v.get<std::string>();
                            using Audio_meter_pos = Channel_settings::Audio_meter::Audio_meter_pos;
                            channel_settings.audio_meter.position = \
                                (pos == "left") ? Audio_meter_pos::Left :
                                (pos == "right") ? Audio_meter_pos::Right :
                                Audio_meter_pos::Right;
                            o_set = true;
                        }
                    }
                }
                else if (k == "status_bar") {
                    if (!v.is_object()) throw Df(jk + div + k + Df::expn_object);
                    for (json::iterator sb_it = v.begin(); sb_it != v.end(); ++sb_it) {
                        const std::string sb_k = sb_it.key();
                        auto sb_v = sb_it.value();

                        if (sb_k == "enabled") {
                            if (!sb_v.is_boolean())
                                throw Df(jk + div + k + div + sb_k + Df::expn_bool);
                            channel_settings.status_bar.enabled = sb_v.get<bool>();
                            o_set = true;
                        }
                        else if (sb_k == "position") {
                            if (!sb_v.is_string())
                                throw Df(jk + div + k + div + sb_k + Df::expn_string);
                            std::string pos = sb_v.get<std::string>();
                            using Status_bar_pos = Channel_settings::Status_bar::Status_bar_pos;
                            channel_settings.status_bar.position = \
                                (pos == "top_left") ? Status_bar_pos::Top_left :
                                (pos == "top_right") ? Status_bar_pos::Top_right :
                                (pos == "left") ? Status_bar_pos::Left :
                                (pos == "right") ? Status_bar_pos::Right :
                                (pos == "bottom_left") ? Status_bar_pos::Bottom_left :
                                (pos == "bottom_right") ? Status_bar_pos::Bottom_right :
                                Status_bar_pos::Top_left;
                            o_set = true;
                        }
                        else if (sb_k == "aspect") {
                            if (!sb_v.is_boolean())
                                throw Df(jk + div + k + div + sb_k + Df::expn_bool);
                            channel_settings.status_bar.aspect = sb_v.get<bool>();
                            o_set = true;
                        }
                        else if (sb_k == "subtitles") {
                            if (!sb_v.is_boolean())
                                throw Df(jk + div + k + div + sb_k + Df::expn_bool);
                            channel_settings.status_bar.subtitles = sb_v.get<bool>();
                            o_set = true;
                        }
                        else if (sb_k == "teletext") {
                            if (!sb_v.is_boolean())
                                throw Df(jk + div + k + div + sb_k + Df::expn_bool);
                            channel_settings.status_bar.teletext = sb_v.get<bool>();
                            o_set = true;
                        }
                        else if (sb_k == "eit") {
                            if (!sb_v.is_boolean())
                                throw Df(jk + div + k + div + sb_k + Df::expn_bool);
                            channel_settings.status_bar.eit = sb_v.get<bool>();
                            o_set = true;
                        }
                        else if (sb_k == "qos") {
                            if (!sb_v.is_boolean())
                                throw Df(jk + div + k + div + sb_k + Df::expn_bool);
                            channel_settings.status_bar.qos = sb_v.get<bool>();
                            o_set = true;
                        }
                        else if (sb_k == "scte35") {
                            if (!sb_v.is_boolean())
                                throw Df(jk + div + k + div + sb_k + Df::expn_bool);
                            channel_settings.status_bar.scte35 = sb_v.get<bool>();
                            o_set = true;
                        }
                    }
                }
                else if (k == "show_border") {
                    if (!v.is_boolean()) throw Df(jk + div + k + Df::expn_bool);
                    channel_settings.show_border = v.get<bool>();
                    o_set = true;
                }
                else if (k == "border_color") {
                    if (!v.is_number()) throw Df(jk + div + k + Df::expn_number);
                    channel_settings.border_color = v.get<uint>();
                    o_set = true;
                }
                else if (k == "show_aspect_border") {
                    if (!v.is_boolean()) throw Df(jk + div + k + Df::expn_bool);
                    channel_settings.show_aspect_border = v.get<bool>();
                    o_set = true;
                }
                else if (k == "aspect_border_color") {
                    if (!v.is_number()) throw Df(jk + div + k + Df::expn_number);
                    channel_settings.aspect_border_color = v.get<uint>();
                    o_set = true;
                }
            }
        }
    }

    if (o_set) set.emit(*this);
}

string
Settings::to_msgpack() const {
    return "todo";
}

void
Settings::of_msgpack(const string&) {
    set.emit(*this);
}
