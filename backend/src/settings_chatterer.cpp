#include "settings.hpp"
#include "validate.hpp"

using namespace std;
using namespace Ats;

/* ---------- QoE settings --------------- */

void
Ats::to_json(json& j, const Settings::QoE::Setting& s) {
    j = {{"cont_en",s.cont_en},
         {"cont",s.cont},
         {"peak_en",s.peak_en},
         {"peak",s.peak}};
}

void
Ats::to_json(json& j, const Settings::QoE::Black& b) {
    j = {{"black",b.black},
         {"luma",b.luma},
         {"time",b.time},
         {"black_pixel",b.black_pixel}};
}

void
Ats::to_json(json& j, const Settings::QoE::Freeze& f) {
    j = {{"freeze",f.freeze},
         {"diff",f.diff},
         {"time",f.time},
         {"pixel_diff",f.pixel_diff}};
}

void
Ats::to_json(json& j, const Settings::QoE::Blocky& b) {
    j = {{"blocky",b.blocky},
         {"time",b.time},
         {"mark_blocks",b.mark_blocks}};
}

void
Ats::to_json(json& j, const Settings::QoE::Silence& s) {
    j = {{"silence",s.silence},
         {"time",s.time}};
}

void
Ats::to_json(json& j, const Settings::QoE::Loudness& l) {
    j = {{"loudness",l.loudness},
         {"time",l.time}};
}

void
Ats::to_json(json& j, const Settings::QoE::Adv& a) {
    j = {{"adv_diff",a.adv_diff},
         {"adv_buf",a.adv_buf}};
}

void
Ats::to_json(json& j, const Settings::QoE::Video& v) {
    j = {{"loss",v.loss},
         {"black",v.black},
         {"freeze",v.freeze},
         {"blocky",v.blocky}};
}

void
Ats::to_json(json& j, const Settings::QoE::Audio& a) {
    j = {{"loss",a.loss},
         {"silence",a.silence},
         {"loudness",a.loudness},
         {"adv",a.adv}};
}

string
Settings::to_string() const {
    string rval = "Qoe settings:\n\t\t";
    return rval;
}

json
Settings::serialize() const {
    json j = json{{"video",qoe.video},
                  {"audio",qoe.audio}};
    return j;
}

void
Settings::deserialize(const json& j) {

    bool o_set = false;

    auto get_setting = [&o_set](json j, std::string name, Settings::QoE::Setting& s) {
        if (j.find(name) != j.end()) {
            json j_setting = j.at(name);
            SET_VALUE_FROM_JSON(j_setting,s,peak_en,bool,o_set);
            SET_VALUE_FROM_JSON(j_setting,s,peak,float,o_set);
            SET_VALUE_FROM_JSON(j_setting,s,cont_en,bool,o_set);
            SET_VALUE_FROM_JSON(j_setting,s,cont,float,o_set);
        }
    };

    if (j.find("video") != j.end()) {
        auto j_video = j.at("video");

        SET_VALUE_FROM_JSON(j_video,qoe.video,loss,float,o_set);

        if (j_video.find("black") != j_video.end()) {
            auto j_black = j_video.at("black");
            get_setting(j_black,"black",qoe.video.black.black);
            get_setting(j_black,"luma",qoe.video.black.luma);
            SET_VALUE_FROM_JSON(j_black,qoe.video.black,time,float,o_set);
            SET_VALUE_FROM_JSON(j_black,qoe.video.black,black_pixel,uint,o_set);
        }

        if (j_video.find("freeze") != j_video.end()) {
            auto j_freeze = j_video.at("freeze");
            get_setting(j_freeze,"freeze",qoe.video.freeze.freeze);
            get_setting(j_freeze,"diff",qoe.video.freeze.diff);
            SET_VALUE_FROM_JSON(j_freeze,qoe.video.freeze,time,float,o_set);
            SET_VALUE_FROM_JSON(j_freeze,qoe.video.freeze,pixel_diff,uint,o_set);
        }
        
        if (j_video.find("blocky") != j_video.end()) {
            auto j_blocky = j_video.at("blocky");
            get_setting(j_blocky,"blocky",qoe.video.blocky.blocky);
            SET_VALUE_FROM_JSON(j_blocky,qoe.video.blocky,time,float,o_set);
            SET_VALUE_FROM_JSON(j_blocky,qoe.video.blocky,mark_blocks,bool,o_set);
        }
    }

    if (j.find("audio") != j.end()) {
        auto j_audio = j.at("audio");

        SET_VALUE_FROM_JSON(j_audio,qoe.audio,loss,float,o_set);

        if (j_audio.find("silence") != j_audio.end()) {
            auto j_silence = j_audio.at("silence");
            get_setting(j_silence,"silence",qoe.audio.silence.silence);
            SET_VALUE_FROM_JSON(j_silence,qoe.audio.silence,time,float,o_set);
        }
        
        if (j_audio.find("loudness") != j_audio.end()) {
            auto j_loudness = j_audio.at("loudness");
            get_setting(j_loudness,"loudness",qoe.audio.loudness.loudness);
            SET_VALUE_FROM_JSON(j_loudness,qoe.audio.loudness,time,float,o_set);
        }
        
        if (j_audio.find("adv") != j_audio.end()) {
            auto j_adv = j_audio.at("adv");
            SET_VALUE_FROM_JSON(j_adv,qoe.audio.adv,adv_diff,float,o_set);
            SET_VALUE_FROM_JSON(j_adv,qoe.audio.adv,adv_buf,uint,o_set);
        }
        
    }

    if (o_set) set.emit(*this);
    send.emit(*this);
}
