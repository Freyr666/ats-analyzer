#include "settings.hpp"
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
    string rval = "Qoe settings:\n\t\t";
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
    constexpr const char* qoe_settings_key = "qoe_settings";
    constexpr const char* channel_settings_key = "channel_settings";

    bool o_set = false;

    /* if qoe settings present in json */
    if (j.find(qoe_settings_key) != j.end()) {
        // loss
        auto j_qoe = j.at(qoe_settings_key);
        set_value_from_json(j_qoe,qoe_settings,vloss,float);
        set_value_from_json(j_qoe,qoe_settings,aloss,float);
        // black frame
        set_value_from_json(j_qoe,qoe_settings,black_cont_en,bool);
        set_value_from_json(j_qoe,qoe_settings,black_cont,float);
        set_value_from_json(j_qoe,qoe_settings,black_peak_en,bool);
        set_value_from_json(j_qoe,qoe_settings,black_peak,float);
        set_value_from_json(j_qoe,qoe_settings,luma_cont_en,bool);
        set_value_from_json(j_qoe,qoe_settings,luma_cont,float);
        set_value_from_json(j_qoe,qoe_settings,luma_peak_en,bool);
        set_value_from_json(j_qoe,qoe_settings,luma_peak,float);
        set_value_from_json(j_qoe,qoe_settings,black_time,float);
        set_value_from_json(j_qoe,qoe_settings,black_pixel,uint);
        // freeze
        set_value_from_json(j_qoe,qoe_settings,freeze_cont_en,bool);
        set_value_from_json(j_qoe,qoe_settings,freeze_cont,float);
        set_value_from_json(j_qoe,qoe_settings,freeze_peak_en,bool);
        set_value_from_json(j_qoe,qoe_settings,freeze_peak,float);
        set_value_from_json(j_qoe,qoe_settings,diff_cont_en,bool);
        set_value_from_json(j_qoe,qoe_settings,diff_cont,float);
        set_value_from_json(j_qoe,qoe_settings,diff_peak_en,bool);
        set_value_from_json(j_qoe,qoe_settings,diff_peak,float);
        set_value_from_json(j_qoe,qoe_settings,freeze_time,float);
        set_value_from_json(j_qoe,qoe_settings,pixel_diff,uint);
        // blockiness
        set_value_from_json(j_qoe,qoe_settings,blocky_cont_en,bool);
        set_value_from_json(j_qoe,qoe_settings,blocky_cont,float);
        set_value_from_json(j_qoe,qoe_settings,blocky_peak_en,bool);
        set_value_from_json(j_qoe,qoe_settings,blocky_peak,float);
        set_value_from_json(j_qoe,qoe_settings,blocky_time,float);
        set_value_from_json(j_qoe,qoe_settings,mark_blocks,bool);
        // silence
        set_value_from_json(j_qoe,qoe_settings,silence_cont_en,bool);
        set_value_from_json(j_qoe,qoe_settings,silence_cont,float);
        set_value_from_json(j_qoe,qoe_settings,silence_peak_en,bool);
        set_value_from_json(j_qoe,qoe_settings,silence_peak,float);
        set_value_from_json(j_qoe,qoe_settings,silence_time,float);
        // loudness
        set_value_from_json(j_qoe,qoe_settings,loudness_cont_en,bool);
        set_value_from_json(j_qoe,qoe_settings,loudness_cont,float);
        set_value_from_json(j_qoe,qoe_settings,loudness_peak_en,bool);
        set_value_from_json(j_qoe,qoe_settings,loudness_peak,float);
        set_value_from_json(j_qoe,qoe_settings,loudness_time,float);
        // adv loudness
        set_value_from_json(j_qoe,qoe_settings,adv_diff,float);
        set_value_from_json(j_qoe,qoe_settings,adv_buf,uint);
        o_set = true;
    } // TODO maybe add log message at else clause

    /* if channel settings present in json */
    if (j.find(channel_settings_key) != j.end()) {
        auto j_channel = j.at(channel_settings_key);
        set_value_from_json(j_channel,channel_settings,show_border,bool);
        set_value_from_json(j_channel,channel_settings,border_color,uint);
        set_value_from_json(j_channel,channel_settings,show_aspect_border,bool);
        set_value_from_json(j_channel,channel_settings,aspect_border_color,uint);
        if (j_channel.find("error_overlay") != j_channel.end()) {
            auto j_eo = j_channel.at("error_overlay");
            set_value_from_json(j_eo,channel_settings.error_overlay,enabled,bool);
            set_value_from_json(j_eo,channel_settings.error_overlay,error_color,uint);
        }
        if (j_channel.find("channel_name") != j_channel.end()) {
            auto j_cn = j_channel.at("channel_name");
            set_value_from_json(j_cn,channel_settings.channel_name,enabled,bool);
            set_value_from_json(j_cn,channel_settings.channel_name,font_size,int);
            set_value_from_json(j_cn,channel_settings.channel_name,fmt,std::string);
        }
        if (j_channel.find("audio_meter") != j_channel.end()) {
            auto j_am = j_channel.at("audio_meter");
            set_value_from_json(j_am,channel_settings.audio_meter,enabled,bool);
            if (j_am.find("position") != j_am.end()) {
                using Audio_meter_pos = Settings::Channel_settings::Audio_meter::Audio_meter_pos;
                std::string pos_str = j_am.at("position").get<std::string>();
                channel_settings.audio_meter.position = \
                    (pos_str == "left" ? Audio_meter_pos::Left :
                     Audio_meter_pos::Right);
            }
        }
        if (j_channel.find("status_bar") != j_channel.end()) {
            auto j_sb = j_channel.at("status_bar");
            set_value_from_json(j_sb,channel_settings.status_bar,enabled,bool);
            if (j_sb.find("position") != j_sb.end()) {
                using Status_bar_pos = Settings::Channel_settings::Status_bar::Status_bar_pos;
                std::string pos_str = j_sb.at("position").get<std::string>();
                channel_settings.status_bar.position = \
                    (pos_str == "top_left" ? Status_bar_pos::Top_left :
                     pos_str == "top_right" ? Status_bar_pos::Top_right :
                     pos_str == "left" ? Status_bar_pos::Left :
                     pos_str == "right" ? Status_bar_pos::Right :
                     pos_str == "bottom_left" ? Status_bar_pos::Bottom_left :
                     Status_bar_pos::Bottom_right);
            }
            set_value_from_json(j_sb,channel_settings.status_bar,aspect,bool);
            set_value_from_json(j_sb,channel_settings.status_bar,subtitles,bool);
            set_value_from_json(j_sb,channel_settings.status_bar,teletext,bool);
            set_value_from_json(j_sb,channel_settings.status_bar,eit,bool);
            set_value_from_json(j_sb,channel_settings.status_bar,qos,bool);
            set_value_from_json(j_sb,channel_settings.status_bar,scte35,bool);
            o_set = true;
        }
    } // TODO maybe add log message at else clause

    if (o_set) set.emit(*this);
}
