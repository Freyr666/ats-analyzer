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

void
Ats::to_json(json& j, const Settings::Qoe_settings::Setting& s) {
    j = {{"cont_en",s.cont_en},
         {"cont",s.cont},
         {"peak_en",s.peak_en},
         {"peak",s.peak}};
}

void
Ats::to_json(json& j, const Settings::Qoe_settings::Loss& l) {
    j = {{"vloss",l.vloss},
         {"aloss",l.aloss}};
}

void
Ats::to_json(json& j, const Settings::Qoe_settings::Black& b) {
    j = {{"black",b.black},
         {"luma",b.luma},
         {"time",b.time},
         {"black_pixel",b.black_pixel}};
}

void
Ats::to_json(json& j, const Settings::Qoe_settings::Freeze& f) {
    j = {{"freeze",f.freeze},
         {"diff",f.diff},
         {"time",f.time},
         {"pixel_diff",f.pixel_diff}};
}

void
Ats::to_json(json& j, const Settings::Qoe_settings::Blocky& b) {
    j = {{"blocky",b.blocky},
         {"time",b.time},
         {"mark_blocks",b.mark_blocks}};
}

void
Ats::to_json(json& j, const Settings::Qoe_settings::Silence& s) {
    j = {{"silence",s.silence},
         {"time",s.time}};
}

void
Ats::to_json(json& j, const Settings::Qoe_settings::Loudness& l) {
    j = {{"loudness",l.loudness},
         {"time",l.time}};
}

void
Ats::to_json(json& j, const Settings::Qoe_settings::Adv& a) {
    j = {{"adv_diff",a.adv_diff},
         {"adv_buf",a.adv_buf}};
}

string
Settings::Qoe_settings::to_string() const {
    auto setting_to_string = [](Setting set) {
        std::string s = "\n\t\tpeak en: ";
        s += Ats::to_string(set.peak_en);
        s += "\n\t\tpeak: ";
        s += std::to_string(set.peak);
        s += "\n\t\tcont_en: ";
        s += Ats::to_string(set.cont_en);
        s += "\n\t\tpeak: ";
        s += std::to_string(set.cont);
        return s;
    };

    string rval = "Loss:";
    rval += "\n\tVloss: "       + std::to_string(loss.vloss);
    rval += "\n\tAloss: "       + std::to_string(loss.aloss);
    rval += "\nBlack:";
    rval += "\n\tBlack: "       + setting_to_string(black.black);
    rval += "\n\tLuma: "        + setting_to_string(black.luma);
    rval += "\n\tTime: "        + std::to_string(black.time);
    rval += "\n\tBlack pixel: " + std::to_string(black.black_pixel);
    rval += "\nFreeze:";
    rval += "\n\tFreeze: "      + setting_to_string(freeze.freeze);
    rval += "\n\tDiff: "        + setting_to_string(freeze.diff);
    rval += "\n\tTime: "        + std::to_string(freeze.time);
    rval += "\n\tPixel diff: "  + std::to_string(freeze.pixel_diff);
    rval += "\nBlocky:";
    rval += "\n\tBlocky: "      + setting_to_string(blocky.blocky);
    rval += "\n\tTime: "        + std::to_string(blocky.time);
    rval += "\n\tMark blocks: " + Ats::to_string(blocky.mark_blocks);
    rval += "\nSilence:";
    rval += "\n\tSilence: "     + setting_to_string(silence.silence);
    rval += "\n\tTime: "        + std::to_string(silence.time);
    rval += "\nLoudness:";
    rval += "\n\tLoudness: "    + setting_to_string(loudness.loudness);
    rval += "\n\tTime: "        + std::to_string(loudness.time);
    rval += "\nAdv:";
    rval += "\n\tAdv diff: "    + std::to_string(adv.adv_diff);
    rval += "\n\tAdv buf: "     + std::to_string(adv.adv_buf);
    rval += "\n";
    return rval;
}

void
Ats::to_json(json& j, const Settings::Qoe_settings& qoe) {
    j = {{"loss",qoe.loss},
         {"black",qoe.black},
         {"freeze",qoe.freeze},
         {"blocky",qoe.blocky},
         {"silence",qoe.silence},
         {"loudness",qoe.loudness},
         {"adv",qoe.adv}};
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
    string rval = "\tQoe settings:\n\t\t";
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

        auto get_setting = [&o_set](json j, std::string name, Settings::Qoe_settings::Setting& s) {
            if (j.find(name) != j.end()) {
                json j_setting = j.at(name);
                SET_VALUE_FROM_JSON(j_setting,s,peak_en,bool,o_set);
                SET_VALUE_FROM_JSON(j_setting,s,peak,float,o_set);
                SET_VALUE_FROM_JSON(j_setting,s,cont_en,bool,o_set);
                SET_VALUE_FROM_JSON(j_setting,s,cont,float,o_set);
            }
        };

        if (j_qoe.find("loss") != j_qoe.end()) {
            auto j_loss = j_qoe.at("loss");
            SET_VALUE_FROM_JSON(j_loss,qoe_settings.loss,vloss,float,o_set);
            SET_VALUE_FROM_JSON(j_loss,qoe_settings.loss,aloss,float,o_set);
        }
        if (j_qoe.find("black") != j_qoe.end()) {
            auto j_black = j_qoe.at("black");
            get_setting(j_black,"black",qoe_settings.black.black);
            get_setting(j_black,"luma",qoe_settings.black.luma);
            SET_VALUE_FROM_JSON(j_black,qoe_settings.black,time,float,o_set);
            SET_VALUE_FROM_JSON(j_black,qoe_settings.black,black_pixel,uint,o_set);
        }
        if (j_qoe.find("freeze") != j_qoe.end()) {
            auto j_freeze = j_qoe.at("freeze");
            get_setting(j_freeze,"freeze",qoe_settings.freeze.freeze);
            get_setting(j_freeze,"diff",qoe_settings.freeze.diff);
            SET_VALUE_FROM_JSON(j_freeze,qoe_settings.freeze,time,float,o_set);
            SET_VALUE_FROM_JSON(j_freeze,qoe_settings.freeze,pixel_diff,uint,o_set);
        }
        if (j_qoe.find("blocky") != j_qoe.end()) {
            auto j_blocky = j_qoe.at("blocky");
            get_setting(j_blocky,"blocky",qoe_settings.blocky.blocky);
            SET_VALUE_FROM_JSON(j_blocky,qoe_settings.blocky,time,float,o_set);
            SET_VALUE_FROM_JSON(j_blocky,qoe_settings.blocky,mark_blocks,bool,o_set);
        }
        if (j_qoe.find("silence") != j_qoe.end()) {
            auto j_silence = j_qoe.at("silence");
            get_setting(j_silence,"silence",qoe_settings.silence.silence);
            SET_VALUE_FROM_JSON(j_silence,qoe_settings.silence,time,float,o_set);
        }
        if (j_qoe.find("loudness") != j_qoe.end()) {
            auto j_loudness = j_qoe.at("loudness");
            get_setting(j_loudness,"loudness",qoe_settings.loudness.loudness);
            SET_VALUE_FROM_JSON(j_loudness,qoe_settings.loudness,time,float,o_set);
        }
        if (j_qoe.find("adv") != j_qoe.end()) {
            auto j_adv = j_qoe.at("adv");
            SET_VALUE_FROM_JSON(j_adv,qoe_settings.adv,adv_diff,float,o_set);
            SET_VALUE_FROM_JSON(j_adv,qoe_settings.adv,adv_buf,uint,o_set);
        }
    } // TODO maybe add log message at else clause

    /* if channel settings present in json */
    if (j.find(channel_settings_key) != j.end()) {
        auto j_channel = j.at(channel_settings_key);
        SET_VALUE_FROM_JSON(j_channel,channel_settings,show_border,bool,o_set);
        SET_VALUE_FROM_JSON(j_channel,channel_settings,border_color,uint,o_set);
        SET_VALUE_FROM_JSON(j_channel,channel_settings,show_aspect_border,bool,o_set);
        SET_VALUE_FROM_JSON(j_channel,channel_settings,aspect_border_color,uint,o_set);
        if (j_channel.find("error_overlay") != j_channel.end()) {
            auto j_eo = j_channel.at("error_overlay");
            SET_VALUE_FROM_JSON(j_eo,channel_settings.error_overlay,enabled,bool,o_set);
            SET_VALUE_FROM_JSON(j_eo,channel_settings.error_overlay,error_color,uint,o_set);
        }
        if (j_channel.find("channel_name") != j_channel.end()) {
            auto j_cn = j_channel.at("channel_name");
            SET_VALUE_FROM_JSON(j_cn,channel_settings.channel_name,enabled,bool,o_set);
            SET_VALUE_FROM_JSON(j_cn,channel_settings.channel_name,font_size,int,o_set);
            SET_VALUE_FROM_JSON(j_cn,channel_settings.channel_name,fmt,std::string,o_set);
        }
        if (j_channel.find("audio_meter") != j_channel.end()) {
            auto j_am = j_channel.at("audio_meter");
            SET_VALUE_FROM_JSON(j_am,channel_settings.audio_meter,enabled,bool,o_set);
            if (j_am.find("position") != j_am.end()) {
                using Audio_meter_pos = Settings::Channel_settings::Audio_meter::Audio_meter_pos;
                std::string pos_str = j_am.at("position").get<std::string>();
                channel_settings.audio_meter.position = \
                    (pos_str == "left" ? Audio_meter_pos::Left :
                     Audio_meter_pos::Right);
                o_set = true;
            }
        }
        if (j_channel.find("status_bar") != j_channel.end()) {
            auto j_sb = j_channel.at("status_bar");
            SET_VALUE_FROM_JSON(j_sb,channel_settings.status_bar,enabled,bool,o_set);
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
                o_set = true;
            }
            SET_VALUE_FROM_JSON(j_sb,channel_settings.status_bar,aspect,bool,o_set);
            SET_VALUE_FROM_JSON(j_sb,channel_settings.status_bar,subtitles,bool,o_set);
            SET_VALUE_FROM_JSON(j_sb,channel_settings.status_bar,teletext,bool,o_set);
            SET_VALUE_FROM_JSON(j_sb,channel_settings.status_bar,eit,bool,o_set);
            SET_VALUE_FROM_JSON(j_sb,channel_settings.status_bar,qos,bool,o_set);
            SET_VALUE_FROM_JSON(j_sb,channel_settings.status_bar,scte35,bool,o_set);
        }
    } // TODO maybe add log message at else clause

    if (o_set) set.emit(*this);
}
