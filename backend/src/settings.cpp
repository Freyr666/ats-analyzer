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
    rval += "\n";
    return rval;
}

json
Settings::serialize() const {
    json j = json{{"qoe_settings", qoe_settings}};
    return j;
}

void
Settings::deserialize(const json& j) {
    constexpr const char* qoe_settings_key = "qoe_settings";

    bool o_set = false;

    /* if qoe settings present in json */
    if (j.find(qoe_settings_key) != j.end()) {
        // loss
        auto j_qoe = j.at(qoe_settings_key);
        SET_VALUE_FROM_JSON(j_qoe,qoe_settings,vloss,float,o_set);
        SET_VALUE_FROM_JSON(j_qoe,qoe_settings,aloss,float,o_set);
        // black frame
        SET_VALUE_FROM_JSON(j_qoe,qoe_settings,black_cont_en,bool,o_set);
        SET_VALUE_FROM_JSON(j_qoe,qoe_settings,black_cont,float,o_set);
        SET_VALUE_FROM_JSON(j_qoe,qoe_settings,black_peak_en,bool,o_set);
        SET_VALUE_FROM_JSON(j_qoe,qoe_settings,black_peak,float,o_set);
        SET_VALUE_FROM_JSON(j_qoe,qoe_settings,luma_cont_en,bool,o_set);
        SET_VALUE_FROM_JSON(j_qoe,qoe_settings,luma_cont,float,o_set);
        SET_VALUE_FROM_JSON(j_qoe,qoe_settings,luma_peak_en,bool,o_set);
        SET_VALUE_FROM_JSON(j_qoe,qoe_settings,luma_peak,float,o_set);
        SET_VALUE_FROM_JSON(j_qoe,qoe_settings,black_time,float,o_set);
        SET_VALUE_FROM_JSON(j_qoe,qoe_settings,black_pixel,uint,o_set);
        // freeze
        SET_VALUE_FROM_JSON(j_qoe,qoe_settings,freeze_cont_en,bool,o_set);
        SET_VALUE_FROM_JSON(j_qoe,qoe_settings,freeze_cont,float,o_set);
        SET_VALUE_FROM_JSON(j_qoe,qoe_settings,freeze_peak_en,bool,o_set);
        SET_VALUE_FROM_JSON(j_qoe,qoe_settings,freeze_peak,float,o_set);
        SET_VALUE_FROM_JSON(j_qoe,qoe_settings,diff_cont_en,bool,o_set);
        SET_VALUE_FROM_JSON(j_qoe,qoe_settings,diff_cont,float,o_set);
        SET_VALUE_FROM_JSON(j_qoe,qoe_settings,diff_peak_en,bool,o_set);
        SET_VALUE_FROM_JSON(j_qoe,qoe_settings,diff_peak,float,o_set);
        SET_VALUE_FROM_JSON(j_qoe,qoe_settings,freeze_time,float,o_set);
        SET_VALUE_FROM_JSON(j_qoe,qoe_settings,pixel_diff,uint,o_set);
        // blockiness
        SET_VALUE_FROM_JSON(j_qoe,qoe_settings,blocky_cont_en,bool,o_set);
        SET_VALUE_FROM_JSON(j_qoe,qoe_settings,blocky_cont,float,o_set);
        SET_VALUE_FROM_JSON(j_qoe,qoe_settings,blocky_peak_en,bool,o_set);
        SET_VALUE_FROM_JSON(j_qoe,qoe_settings,blocky_peak,float,o_set);
        SET_VALUE_FROM_JSON(j_qoe,qoe_settings,blocky_time,float,o_set);
        SET_VALUE_FROM_JSON(j_qoe,qoe_settings,mark_blocks,bool,o_set);
        // silence
        SET_VALUE_FROM_JSON(j_qoe,qoe_settings,silence_cont_en,bool,o_set);
        SET_VALUE_FROM_JSON(j_qoe,qoe_settings,silence_cont,float,o_set);
        SET_VALUE_FROM_JSON(j_qoe,qoe_settings,silence_peak_en,bool,o_set);
        SET_VALUE_FROM_JSON(j_qoe,qoe_settings,silence_peak,float,o_set);
        SET_VALUE_FROM_JSON(j_qoe,qoe_settings,silence_time,float,o_set);
        // loudness
        SET_VALUE_FROM_JSON(j_qoe,qoe_settings,loudness_cont_en,bool,o_set);
        SET_VALUE_FROM_JSON(j_qoe,qoe_settings,loudness_cont,float,o_set);
        SET_VALUE_FROM_JSON(j_qoe,qoe_settings,loudness_peak_en,bool,o_set);
        SET_VALUE_FROM_JSON(j_qoe,qoe_settings,loudness_peak,float,o_set);
        SET_VALUE_FROM_JSON(j_qoe,qoe_settings,loudness_time,float,o_set);
        // adv loudness
        SET_VALUE_FROM_JSON(j_qoe,qoe_settings,adv_diff,float,o_set);
        SET_VALUE_FROM_JSON(j_qoe,qoe_settings,adv_buf,uint,o_set);
    } // TODO maybe add log message at else clause
    
    if (o_set) set.emit(*this);
}
