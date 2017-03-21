#include "options.hpp"

#include <cstdio>
#include <algorithm>
#include <iostream>

using namespace std;
using namespace Ats;

/* ---------- Channel settings ----------- */

string
Options::Channel_settings::Error_overlay::to_json() const {
    #define SIZE 1024

    char buffer[SIZE];
    std::string fmt = "{ "
        "\"error_color\": \"%s\", "
        "\"blink_speed\": %f, "
        "\"enabled\": %s"
        " }";

    int n = snprintf (buffer, SIZE, fmt.c_str(),
                      error_color.c_str(),
                      blink_speed,
                      enabled ? "true" : "false");
    if ( (n>=0) && (n<SIZE) ) return string(buffer);
    else return "{}";
}

string
Options::Channel_settings::Channel_name::to_json() const {
    #define SIZE 1024

    char buffer[SIZE];
    std::string fmt = "{ "
        "\"position\": \"%s\", "
        "\"font_size\": %d, "
        "\"fmt\": \"%s\", "
        "\"fullscreen\": %s"
        " }";

    int n = snprintf (buffer, SIZE, fmt.c_str(),
                      position == Channel_name_pos::Left ? "left" :
                      position == Channel_name_pos::Right ? "right" :
                      position == Channel_name_pos::Center ? "center" :
                      "off",
                      font_size,
                      this->fmt.c_str(),
                      fullscreen ? "true" : "false");
    if ( (n>=0) && (n<SIZE) ) return string(buffer);
    else return "{}";
}

string
Options::Channel_settings::Audio_meter::to_json() const {
#define SIZE 1024

    char buffer[SIZE];
    std::string fmt = "{ "
        "\"position\": \"%s\", "
        "\"overlay\": %s, "
        "\"width\": %d, "
        "\"height\": %d, "
        "\"peak_color\": \"%s\", "
        "\"high_color\": \"%s\", "
        "\"mid_color\": \"%s\", "
        "\"low_color\": \"%s\", "
        "\"background_color\": \"%s\""
        " }";

    int n = snprintf (buffer, SIZE, fmt.c_str(),
                      position == Audio_meter_pos::Left ? "left" :
                      position == Audio_meter_pos::Right ? "right" :
                      "off",
                      overlay ? "true" : "false",
                      width, height,
                      peak_color.c_str(),
                      high_color.c_str(),
                      mid_color.c_str(),
                      low_color.c_str(),
                      background_color.c_str());
    if ( (n>=0) && (n<SIZE) ) return string(buffer);
    else return "{}";
}

string
Options::Channel_settings::Status_bar::to_json() const {
    #define SIZE 1024

    char buffer[SIZE];
    std::string fmt = "{ "
        "\"position\": \"%s\", "
        "\"aspect\": %s, "
        "\"subtitles\": %s, "
        "\"teletext\": %s, "
        "\"eit\": %s, "
        "\"qos\": %s, "
        "\"scte35\": %s"
        " }";

    int n = snprintf (buffer, SIZE, fmt.c_str(),
                      position == Status_bar_pos::Top_left ? "top_left" :
                      position == Status_bar_pos::Top_right ? "top_right" :
                      position == Status_bar_pos::Left ? "left" :
                      position == Status_bar_pos::Right ? "right" :
                      position == Status_bar_pos::Bottom_left ? "bottom_left" :
                      position == Status_bar_pos::Bottom_right ? "bottom_right" :
                      "off",
                      aspect ? "true" : "false",
                      subtitles ? "true" : "false",
                      teletext ? "true" : "false",
                      eit ? "true" : "false",
                      qos ? "true" : "false",
                      scte35 ? "true" : "false");
    if ( (n>=0) && (n<SIZE) ) return string(buffer);
    else return "{}";
}

string
Options::Channel_settings::to_json() const {
    #define SIZE 1024

    char buffer[SIZE];
    std::string fmt = "{ "
        "\"error_overlay\": %s, "
        "\"channel_name\": %s, "
        "\"audio_meter\": %s, "
        "\"status_bar\": %s, "
        "\"show_border\": %s, "
        "\"border_color\": \"%s\", "
        "\"show_aspect_border\": %s, "
        "\"aspect_border_color\": \"%s\""
        " }";

    int n = snprintf (buffer, SIZE, fmt.c_str(),
                      error_overlay.to_json().c_str(),
                      channel_name.to_json().c_str(),
                      audio_meter.to_json().c_str(),
                      status_bar.to_json().c_str(),
                      show_border ? "true" : "false",
                      border_color.c_str(),
                      show_aspect_border ? "true" : "false",
                      aspect_border_color.c_str());
    if ( (n>=0) && (n<SIZE) ) return string(buffer);
    else return "{}";
}

/* ---------- QoE settings --------------- */

string
Options::Qoe_settings::to_json() const {
    #define SIZE 1024

    char buffer[SIZE];
    std::string fmt = "{ "
        "\"vloss\": %f, "
        "\"aloss\": %f, "
        "\"black_cont_en\": %s, "
        "\"black_cont\": %f, "
        "\"black_peak_en\": %s, "
        "\"black_peak\": %f, "
        "\"luma_cont_en\": %s, "
        "\"luma_cont\": %f, "
        "\"luma_peak_en\": %s, "
        "\"luma_peak\": %f, "
        "\"black_time\": %f, "
        "\"black_pixel\": %d, "
        "\"freeze_cont_en\": %s, "
        "\"freeze_cont\": %f, "
        "\"freeze_peak_en\": %s, "
        "\"freeze_peak\": %f, "
        "\"diff_cont_en\": %s, "
        "\"diff_cont\": %f, "
        "\"diff_peak_en\": %s, "
        "\"diff_peak\": %f, "
        "\"freeze_time\": %f, "
        "\"pixel_diff\": %d, "
        "\"blocky_cont_en\": %s, "
        "\"blocky_cont\": %f, "
        "\"blocky_peak_en\": %s, "
        "\"blocky_peak\": %f, "
        "\"blocky_time\": %f, "
        "\"mark_blocks\": %s, "
        "\"silence_cont_en\": %s, "
        "\"silence_cont\": %f, "
        "\"silence_peak_en\": %s, "
        "\"silence_peak\": %f, "
        "\"silence_time\": %f, "
        "\"loudness_cont_en\": %s, "
        "\"loudness_cont\": %f, "
        "\"loudness_peak_en\": %s, "
        "\"loudness_peak\": %f, "
        "\"loudness_time\": %f, "
        "\"adv_diff\": %f, "
        "\"adv_buff\": %d"
        " }";

    // int n = snprintf (buffer, SIZE, fmt.c_str(),
    //                   error_overlay.to_json().c_str(),
    //                   channel_name.to_json().c_str(),
    //                   audio_meter.to_json().c_str(),
    //                   status_bar.to_json().c_str(),
    //                   show_border ? "true" : "false",
    //                   border_color.c_str(),
    //                   show_aspect_border ? "true" : "false",
    //                   aspect_border_color.c_str());
    // if ( (n>=0) && (n<SIZE) ) return string(buffer);
    // else return "{}";

    return "{}";
}

/* ---------- Options -------------------- */

void
Options::set_data(const Metadata& m) {
    if (data.empty()) {
	data.push_back(Metadata(m));
    } else {
	auto v = find_if(data.begin(),data.end(),[&m](Metadata& el) {
		return el.stream == m.stream;
	    });

	if (v->stream == m.stream) {
	    *v = m;
	} else {
	    data.push_back(Metadata(m));
	}
    }
    updated.emit(*this);
}

string
Options::to_string() const {
    // string rval = "\tOptions:\n\tStreams:\n";
    // for_each(data.begin(),data.end(),[&rval](const Metadata& m){
    //         rval += "\n";
    //         rval += m.to_string();
    //         rval += "\n";
    //     });
    // rval += "\tOther options:\n";
    // rval += "Dummy: ";
    // rval += std::to_string(resolution.first);
    // rval += "\n";

    string rval = "Options testing:\n";
    rval += "Channel settings:\n";
    rval += channel_settings.to_json();

    rval += "\n";

    return rval;
}

string
Options::to_json() const {
    return "TODO";
}

void
of_json(const string&) {
    
}
