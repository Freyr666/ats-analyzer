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
        if (el.key() == "qoe_settings" && el.value().is_object()) {
            auto j = el.value();
            for (json::iterator it = j.begin(); it != j.end(); ++it) {
                const std::string k = it.key();
                auto v = it.value();

                if (k == "vloss") {
                    if (!v.is_number()) return; // FIXME and in other returns too
                    qoe_settings.vloss = v.get<float>();
                    o_set = true;
                }
                else if (k == "aloss") {
                    if (!v.is_number()) return;
                    qoe_settings.aloss = v.get<float>();
                    o_set = true;
                }
                else if (k == "black_cont_en") {
                    if (!v.is_boolean()) return;
                    qoe_settings.black_cont_en = v.get<bool>();
                    o_set = true;
                }
                else if (k == "black_cont") {
                    if (!v.is_number()) return;
                    qoe_settings.black_cont = v.get<float>();
                    o_set = true;
                }
                else if (k == "black_peak_en") {
                    if (!v.is_boolean()) return;
                    qoe_settings.black_peak_en = v.get<bool>();
                    o_set = true;
                }
                else if (k == "black_peak") {
                    if (!v.is_number()) return;
                    qoe_settings.black_peak = v.get<float>();
                    o_set = true;
                }
                else if (k == "luma_cont_en") {
                    if (!v.is_boolean()) return;
                    qoe_settings.luma_cont_en = v.get<bool>();
                    o_set = true;
                }
                else if (k == "luma_cont") {
                    if (!v.is_number()) return;
                    qoe_settings.luma_cont = v.get<float>();
                    o_set = true;
                }
                else if (k == "luma_peak_en") {
                    if (!v.is_boolean()) return;
                    qoe_settings.luma_peak_en = v.get<bool>();
                    o_set = true;
                }
                else if (k == "luma_peak") {
                    if (!v.is_number()) return;
                    qoe_settings.luma_peak = v.get<float>();
                    o_set = true;
                }
                else if (k == "black_time") {
                    if (!v.is_number()) return;
                    qoe_settings.black_time = v.get<float>();
                    o_set = true;
                }
                else if (k == "black_pixel") {
                    if (!v.is_number()) return;
                    qoe_settings.black_pixel = v.get<int>();
                    o_set = true;
                }
                else if (k == "freeze_cont_en") {
                    if (!v.is_boolean()) return;
                    qoe_settings.freeze_cont_en = v.get<bool>();
                    o_set = true;
                }
                else if (k == "freeze_cont") {
                    if (!v.is_number()) return;
                    qoe_settings.freeze_cont = v.get<float>();
                    o_set = true;
                }
                else if (k == "freeze_peak_en") {
                    if (!v.is_boolean()) return;
                    qoe_settings.freeze_peak_en = v.get<bool>();
                    o_set = true;
                }
                else if (k == "freeze_peak") {
                    if (!v.is_number()) return;
                    qoe_settings.freeze_peak = v.get<float>();
                    o_set = true;
                }
                else if (k == "diff_cont_en") {
                    if (!v.is_boolean()) return;
                    qoe_settings.diff_cont_en = v.get<bool>();
                    o_set = true;
                }
                else if (k == "diff_cont") {
                    if (!v.is_number()) return;
                    qoe_settings.diff_cont = v.get<float>();
                    o_set = true;
                }
                else if (k == "diff_peak_en") {
                    if (!v.is_boolean()) return;
                    qoe_settings.diff_peak_en = v.get<bool>();
                    o_set = true;
                }
                else if (k == "diff_peak") {
                    if (!v.is_number()) return;
                    qoe_settings.diff_peak = v.get<float>();
                    o_set = true;
                }
                else if (k == "freeze_time") {
                    if (!v.is_number()) return;
                    qoe_settings.freeze_time = v.get<float>();
                    o_set = true;
                }
                else if (k == "pixel_diff") {
                    if (!v.is_number()) return;
                    qoe_settings.pixel_diff = v.get<int>();
                    o_set = true;
                }
                else if (k == "blocky_cont_en") {
                    if (!v.is_boolean()) return;
                    qoe_settings.blocky_cont_en = v.get<bool>();
                    o_set = true;
                }
                else if (k == "blocky_cont") {
                    if (!v.is_number()) return;
                    qoe_settings.blocky_cont = v.get<float>();
                    o_set = true;
                }
                else if (k == "blocky_peak_en") {
                    if (!v.is_boolean()) return;
                    qoe_settings.blocky_peak_en = v.get<bool>();
                    o_set = true;
                }
                else if (k == "blocky_peak") {
                    if (!v.is_number()) return;
                    qoe_settings.blocky_peak = v.get<float>();
                    o_set = true;
                }
                else if (k == "blocky_time") {
                    if (!v.is_number()) return;
                    qoe_settings.blocky_time = v.get<float>();
                    o_set = true;
                }
                else if (k == "mark_blocks") {
                    if (!v.is_boolean()) return;
                    qoe_settings.mark_blocks = v.get<bool>();
                    o_set = true;
                }
                else if (k == "silence_cont_en") {
                    if (!v.is_boolean()) return;
                    qoe_settings.silence_cont_en = v.get<bool>();
                    o_set = true;
                }
                else if (k == "silence_cont") {
                    if (!v.is_number()) return;
                    qoe_settings.silence_cont = v.get<float>();
                    o_set = true;
                }
                else if (k == "silence_peak_en") {
                    if (!v.is_boolean()) return;
                    qoe_settings.silence_peak_en = v.get<bool>();
                    o_set = true;
                }
                else if (k == "silence_peak") {
                    if (!v.is_number()) return;
                    qoe_settings.silence_peak = v.get<float>();
                    o_set = true;
                }
                else if (k == "silence_time") {
                    if (!v.is_number()) return;
                    qoe_settings.silence_time = v.get<float>();
                    o_set = true;
                }
                else if (k == "loudness_cont_en") {
                    if (!v.is_boolean()) return;
                    qoe_settings.loudness_cont_en = v.get<bool>();
                    o_set = true;
                }
                else if (k == "loudness_cont") {
                    if (!v.is_number()) return;
                    qoe_settings.loudness_cont = v.get<float>();
                    o_set = true;
                }
                else if (k == "loudness_peak_en") {
                    if (!v.is_boolean()) return;
                    qoe_settings.loudness_peak_en = v.get<bool>();
                    o_set = true;
                }
                else if (k == "loudness_peak") {
                    if (!v.is_number()) return;
                    qoe_settings.loudness_peak = v.get<float>();
                    o_set = true;
                }
                else if (k == "loudness_time") {
                    if (!v.is_number()) return;
                    qoe_settings.loudness_time = v.get<float>();
                    o_set = true;
                }
                else if (k == "adv_diff") {
                    if (!v.is_number()) return;
                    qoe_settings.adv_diff = v.get<float>();
                    o_set = true;
                }
                else if (k == "adv_buf") {
                    if (!v.is_number()) return;
                    qoe_settings.adv_buf = v.get<int>();
                    o_set = true;
                }
            }
        }
        else if (el.key() == "channel_settings" && el.value().is_object()) {
            auto j = el.value();
            for (json::iterator it = j.begin(); it != j.end(); ++it) {
                const std::string k = it.key();
                auto v = it.value();

                if (k == "error_overlay") {
                    for (json::iterator eo_it = v.begin(); eo_it != v.end(); ++eo_it) {
                        const std::string eo_k = eo_it.key();
                        auto eo_v = eo_it.value();
                        if (eo_k == "enabled") {
                            if (!eo_v.is_boolean()) return;
                            channel_settings.error_overlay.enabled = eo_v.get<bool>();
                            o_set = true;
                        }
                        else if (eo_k == "error_color") {
                            if (!eo_v.is_number()) return;
                            channel_settings.error_overlay.error_color = eo_v.get<int>();
                            o_set = true;
                        }
                    }
                }
                else if (k == "channel_name") {
                    for (json::iterator cn_it = v.begin(); cn_it != v.end(); ++cn_it) {
                        const std::string cn_k = cn_it.key();
                        auto cn_v = cn_it.value();
                        if (cn_k == "enabled") {
                            if (!cn_v.is_boolean()) return;
                            channel_settings.channel_name.enabled = cn_v.get<bool>();
                            o_set = true;
                        }
                        else if (cn_k == "font_size") {
                            if (!cn_v.is_number()) return;
                            channel_settings.channel_name.font_size = cn_v.get<int>();
                            o_set = true;
                        }
                        else if (cn_k == "fmt") {
                            if (!cn_v.is_string()) return;
                            channel_settings.channel_name.fmt = cn_v.get<std::string>();
                            o_set = true;
                        }
                    }
                }
                else if (k == "audio_meter") {
                    for (json::iterator am_it = v.begin(); am_it != v.end(); ++am_it) {
                        const std::string am_k = am_it.key();
                        auto am_v = am_it.value();
                        if (am_k == "enabled") {
                            if (!am_v.is_boolean()) return;
                            channel_settings.audio_meter.enabled = am_v.get<bool>();
                            o_set = true;
                        }
                        else if (am_k == "position") {
                            if (!am_v.is_string()) return;
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
                    for (json::iterator sb_it = v.begin(); sb_it != v.end(); ++sb_it) {
                        const std::string sb_k = sb_it.key();
                        auto sb_v = sb_it.value();
                        if (sb_k == "enabled") {
                            if (!sb_v.is_boolean()) return;
                            channel_settings.status_bar.enabled = sb_v.get<bool>();
                            o_set = true;
                        }
                        else if (sb_k == "position") {
                            if (!sb_v.is_string()) return;
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
                            if (!sb_v.is_boolean()) return;
                            channel_settings.status_bar.aspect = sb_v.get<bool>();
                            o_set = true;
                        }
                        else if (sb_k == "subtitles") {
                            if (!sb_v.is_boolean()) return;
                            channel_settings.status_bar.subtitles = sb_v.get<bool>();
                            o_set = true;
                        }
                        else if (sb_k == "teletext") {
                            if (!sb_v.is_boolean()) return;
                            channel_settings.status_bar.teletext = sb_v.get<bool>();
                            o_set = true;
                        }
                        else if (sb_k == "eit") {
                            if (!sb_v.is_boolean()) return;
                            channel_settings.status_bar.eit = sb_v.get<bool>();
                            o_set = true;
                        }
                        else if (sb_k == "qos") {
                            if (!sb_v.is_boolean()) return;
                            channel_settings.status_bar.qos = sb_v.get<bool>();
                            o_set = true;
                        }
                        else if (sb_k == "scte35") {
                            if (!sb_v.is_boolean()) return;
                            channel_settings.status_bar.scte35 = sb_v.get<bool>();
                            o_set = true;
                        }
                    }
                }
                else if (k == "show_border") {
                    if (!v.is_boolean()) return;
                    channel_settings.show_border = v.get<bool>();
                    o_set = true;
                }
                else if (k == "border_color") {
                    if (!v.is_number()) return;
                    channel_settings.border_color = v.get<int>();
                    o_set = true;
                }
                else if (k == "show_aspect_border") {
                    if (!v.is_boolean()) return;
                    channel_settings.show_aspect_border = v.get<bool>();
                    o_set = true;
                }
                else if (k == "aspect_border_color") {
                    if (!v.is_number()) return;
                    channel_settings.aspect_border_color = v.get<int>();
                    o_set = true;
                }
            }
        }
        else if (el.key() == "output_sink_settings" && el.value().is_object()) {
            auto j = el.value();
            for (json::iterator it = j.begin(); it != j.end(); ++it) {
                const std::string k = it.key();
                auto v = it.value();
                if (k == "enabled") {
                    if (!v.is_boolean()) return;
                    output_sink_settings.enabled = v.get<bool>();
                    o_set = true;
                }
                else if (k == "address") {
                    if (!v.is_string()) return;
                    output_sink_settings.address = v.get<std::string>();
                    o_set = true;
                }
                else if (k == "port") {
                    if (!v.is_number()) return;
                    output_sink_settings.port = v.get<int>();
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
