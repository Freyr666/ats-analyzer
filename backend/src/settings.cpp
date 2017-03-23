#include "settings.hpp"
#include "json.hpp"

#include <cstdio>
#include <algorithm>
#include <iostream>

using namespace std;
using namespace Ats;

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

string
Settings::Channel_settings::Error_overlay::to_json() const {
    constexpr int size = 128;

    char buffer[size];
    constexpr const char* fmt = "{"
        "\"enabled\":%s,"
        "\"error_color\":%d"
        "}";

    int n = snprintf (buffer, size, fmt, Ats::to_string(enabled).c_str(), error_color);
    if ( (n>=0) && (n<size) ) return string(buffer);
    else throw Serializer_failure ();
}

void
Settings::Channel_settings::Error_overlay::of_json(const string& j) {

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

string
Settings::Channel_settings::Channel_name::to_json() const {
    constexpr int size = 256;

    char buffer[size];
    constexpr const char* fmt = "{"
        "\"enabled\":%s,"
        "\"font_size\":%d,"
        "\"fmt\":\"%s\""
        "}";

    int n = snprintf (buffer, size, fmt, Ats::to_string(enabled).c_str(), font_size, this->fmt.c_str());
    if ( (n>=0) && (n<size) ) return string(buffer);
    else throw Serializer_failure ();
}

void
Settings::Channel_settings::Channel_name::of_json(const string& j) {

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

string
Settings::Channel_settings::Audio_meter::to_json() const {
    constexpr int size = 128;

    char buffer[size];
    constexpr const char* fmt = "{"
        "\"enabled\":%s,"
        "\"position\":\"%s\""
        "}";

    int n = snprintf (buffer, size, fmt,
                      Ats::to_string(enabled).c_str(),
                      position == Audio_meter_pos::Left ? "left" :
                      position == Audio_meter_pos::Right ? "right" :
                      "right");
    if ( (n>=0) && (n<size) ) return string(buffer);
    else throw Serializer_failure ();
}

void
Settings::Channel_settings::Audio_meter::of_json(const string& j) {

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

string
Settings::Channel_settings::Status_bar::to_json() const {
    constexpr int size = 1024;

    char buffer[size];
    constexpr const char* fmt = "{"
        "\"enabled\":%s,"
        "\"position\":\"%s\","
        "\"aspect\":%s,"
        "\"subtitles\":%s,"
        "\"teletext\":%s,"
        "\"eit\":%s,"
        "\"qos\":%s,"
        "\"scte35\":%s"
        "}";

    int n = snprintf (buffer, size, fmt,
                      Ats::to_string(enabled).c_str(),
                      position == Status_bar_pos::Top_left ? "top_left" :
                      position == Status_bar_pos::Top_right ? "top_right" :
                      position == Status_bar_pos::Left ? "left" :
                      position == Status_bar_pos::Right ? "right" :
                      position == Status_bar_pos::Bottom_left ? "bottom_left" :
                      position == Status_bar_pos::Bottom_right ? "bottom_right" :
                      "top_left",
                      Ats::to_string(aspect).c_str(),
                      Ats::to_string(subtitles).c_str(),
                      Ats::to_string(teletext).c_str(),
                      Ats::to_string(eit).c_str(),
                      Ats::to_string(qos).c_str(),
                      Ats::to_string(scte35).c_str());
    if ( (n>=0) && (n<size) ) return string(buffer);
    else throw Serializer_failure ();
}

void
Settings::Channel_settings::Status_bar::of_json(const string& j) {

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

string
Settings::Channel_settings::to_json() const {
    constexpr int size = 1024;

    char buffer[size];
    constexpr const char* fmt = "{"
        "\"show_border\":%s,"
        "\"border_color\":%d,"
        "\"show_aspect_border\":%s,"
        "\"aspect_border_color\":%d,"
        "\"error_overlay\":%s,"
        "\"channel_name\":%s,"
        "\"audio_meter\":%s,"
        "\"status_bar\":%s"
        "}";

    int n = snprintf (buffer, size, fmt,
                      Ats::to_string(show_border).c_str(),
                      border_color,
                      Ats::to_string(show_aspect_border).c_str(),
                      aspect_border_color,
                      error_overlay.to_json().c_str(),
                      channel_name.to_json().c_str(),
                      audio_meter.to_json().c_str(),
                      status_bar.to_json().c_str());
    if ( (n>=0) && (n<size) ) return string(buffer);
    else throw Serializer_failure ();
}

void
Settings::Channel_settings::of_json(const string& j) {

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

string
Settings::Qoe_settings::to_json() const {
    constexpr int size = 1024;

    char buffer[size];
    constexpr const char* fmt = "{"
        "\"vloss\":%.2f,"
        "\"aloss\":%.2f,"
        "\"black_cont_en\":%s,"
        "\"black_cont\":%.2f,"
        "\"black_peak_en\":%s,"
        "\"black_peak\":%.2f,"
        "\"luma_cont_en\":%s,"
        "\"luma_cont\":%.2f,"
        "\"luma_peak_en\":%s,"
        "\"luma_peak\":%.2f,"
        "\"black_time\":%.2f,"
        "\"black_pixel\":%d,"
        "\"freeze_cont_en\":%s,"
        "\"freeze_cont\":%.2f,"
        "\"freeze_peak_en\":%s,"
        "\"freeze_peak\":%.2f,"
        "\"diff_cont_en\":%s,"
        "\"diff_cont\":%.2f,"
        "\"diff_peak_en\":%s,"
        "\"diff_peak\":%.2f,"
        "\"freeze_time\":%.2f,"
        "\"pixel_diff\":%d,"
        "\"blocky_cont_en\":%s,"
        "\"blocky_cont\":%.2f,"
        "\"blocky_peak_en\":%s,"
        "\"blocky_peak\":%.2f,"
        "\"blocky_time\":%.2f,"
        "\"mark_blocks\":%s,"
        "\"silence_cont_en\":%s,"
        "\"silence_cont\":%.2f,"
        "\"silence_peak_en\":%s,"
        "\"silence_peak\":%.2f,"
        "\"silence_time\":%.2f,"
        "\"loudness_cont_en\":%s,"
        "\"loudness_cont\":%.2f,"
        "\"loudness_peak_en\":%s,"
        "\"loudness_peak\":%.2f,"
        "\"loudness_time\":%.2f,"
        "\"adv_diff\":%.2f,"
        "\"adv_buf\":%d"
        "}";

    int n = snprintf (buffer, size, fmt,
                      vloss, aloss,
                      Ats::to_string(black_cont_en).c_str(), black_cont,
                      Ats::to_string(black_peak_en).c_str(), black_peak,
                      Ats::to_string(luma_cont_en).c_str(), luma_cont,
                      Ats::to_string(luma_peak_en).c_str(), luma_peak,
                      black_time, black_pixel,
                      Ats::to_string(freeze_cont_en).c_str(), freeze_cont,
                      Ats::to_string(freeze_peak_en).c_str(), freeze_peak,
                      Ats::to_string(diff_cont_en).c_str(), diff_cont,
                      Ats::to_string(diff_peak_en).c_str(), diff_peak,
                      freeze_time, pixel_diff,
                      Ats::to_string(blocky_cont_en).c_str(), blocky_cont,
                      Ats::to_string(blocky_peak_en).c_str(), blocky_peak,
                      blocky_time, Ats::to_string(mark_blocks).c_str(),
                      Ats::to_string(silence_cont_en).c_str(), silence_cont,
                      Ats::to_string(silence_peak_en).c_str(), silence_peak,
                      silence_time,
                      Ats::to_string(loudness_cont_en).c_str(), loudness_cont,
                      Ats::to_string(loudness_peak_en).c_str(), loudness_peak,
                      loudness_time,
                      adv_diff, adv_buf);
    if ( (n>=0) && (n<size) ) return string(buffer);
    else throw Serializer_failure ();
}

void
Settings::Qoe_settings::of_json(const string& j) {

}

/* ---------- Output sink settings --------------- */

string
Settings::Output_sink::to_string() const {
    string rval = "Enabled: ";
    rval += Ats::to_string(enabled);
    rval += ", Address: ";
    rval += address;
    rval += ", Port: ";
    rval += std::to_string(port);
    rval += "\n";
    return rval;
}

string
Settings::Output_sink::to_json() const {
    constexpr int size = 256;

    char buffer[size];
    constexpr const char* fmt = "{"
        "\"enabled\":%s,"
        "\"address\":\"%s\","
        "\"port\":%d"
        "}";

    int n = snprintf (buffer, size, fmt, Ats::to_string(enabled).c_str(), address.c_str(), port);
    if ( (n>=0) && (n<size) ) return string(buffer);
    else throw Serializer_failure ();
}

void
Settings::Output_sink::of_json(const string& j) {

}


/* ---------- Settings -------------------- */

// Chatter implementation

string
Settings::to_string() const {
    string rval = "Settings:\n\tQoe settings:\n\t\t";
    string qoe_string = qoe_settings.to_string();
    rval += Ats::add_indent(qoe_string);
    rval += "\tChannel_settings:\n\t\t";
    string channel_string = channel_settings.to_string();
    rval += Ats::add_indent(channel_string);
    rval += "\tOutput sink settings:\n\t\t";
    string output_sink_string = output_sink_settings.to_string();
    rval += Ats::add_indent(output_sink_string);
    rval += "\n";
    return rval;
}

string
Settings::to_json() const {
    constexpr int size = 1024 * 8;

    char buffer[size];
    constexpr const char* fmt = "{"
        "\"qoe_settings\":%s,"
        "\"channel_settings\":%s,"
        "\"output_sink\":%s"
        "}";

    int n = snprintf (buffer, size, fmt,
                      qoe_settings.to_json().c_str(),
                      channel_settings.to_json().c_str(),
                      output_sink_settings.to_json().c_str());
    if ( (n>=0) && (n<size) ) return string(buffer);
    else throw Serializer_failure ();
}

void
Settings::of_json(const string& j) {
    bool o_set = false;
    
    using json = nlohmann::json;
    auto js = json::parse(j);

    // TODO throw Wrong_json
    if (! js.is_object()) return;

    for (json::iterator el = js.begin(); el != js.end(); ++el) {
        if (el.key() == "option" && el.value().is_string()) {
            o_set = true; // not so serious option
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
