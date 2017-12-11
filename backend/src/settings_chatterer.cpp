#include "settings.hpp"
#include "validate.hpp"

using namespace std;
using namespace Ats;

/* ---------- QoE settings --------------- */

void
Ats::to_json(json& j, const Settings::Setting& s) {
    j = {{"cont_en",s.cont_en},
         {"cont",s.cont},
         {"peak_en",s.peak_en},
         {"peak",s.peak},
         {"duration",s.duration}};
}

void
Ats::to_json(json& j, const Settings::Black& b) {
    j = {{"black",b.black},
         {"luma",b.luma},
         {"black_pixel",b.black_pixel}};
}

void
Ats::to_json(json& j, const Settings::Freeze& f) {
    j = {{"freeze",f.freeze},
         {"diff",f.diff},
         {"pixel_diff",f.pixel_diff}};
}

void
Ats::to_json(json& j, const Settings::Blocky& b) {
    j = {{"blocky",b.blocky},
         {"mark_blocks",b.mark_blocks}};
}

void
Ats::to_json(json& j, const Settings::Silence& s) {
    j = {{"silence",s.silence}};
}

void
Ats::to_json(json& j, const Settings::Loudness& l) {
    j = {{"loudness",l.loudness}};
}

void
Ats::to_json(json& j, const Settings::Adv& a) {
    j = {{"adv_diff",a.adv_diff},
         {"adv_buf",a.adv_buf}};
}

void
Ats::to_json(json& j, const Settings::Video& v) {
    j = {{"loss",v.loss},
         {"black",v.black},
         {"freeze",v.freeze},
         {"blocky",v.blocky}};
}

void
Ats::to_json(json& j, const Settings::Audio& a) {
    j = {{"loss",a.loss},
         {"silence",a.silence},
         {"loudness",a.loudness},
         {"adv",a.adv}};
}

string
Settings_facade::to_string() const {
    string rval = "Qoe settings:\n\t\t";
    return rval;
}

json
Settings_facade::serialize() const {
    json j = json{{"video",settings.video},
                  {"audio",settings.audio}};
    return j;
}

void
Settings_facade::deserialize(const json& j) {

    bool o_set = false;

    auto get_setting = [&o_set](json j, std::string name, Settings::Setting& s) {
        if (j.find(name) != j.end()) {
            json j_setting = j.at(name);
            SET_VALUE_FROM_JSON(j_setting,s,peak_en,bool,o_set);
            SET_VALUE_FROM_JSON(j_setting,s,peak,float,o_set);
            SET_VALUE_FROM_JSON(j_setting,s,cont_en,bool,o_set);
            SET_VALUE_FROM_JSON(j_setting,s,cont,float,o_set);
            SET_VALUE_FROM_JSON(j_setting,s,duration,float,o_set);
        }
    };

    if (j.find("video") != j.end()) {
        auto j_video = j.at("video");

        SET_VALUE_FROM_JSON(j_video,settings.video,loss,float,o_set);

        if (j_video.find("black") != j_video.end()) {
            auto j_black = j_video.at("black");
            get_setting(j_black,"black",settings.video.black.black);
            get_setting(j_black,"luma",settings.video.black.luma);
            SET_VALUE_FROM_JSON(j_black,settings.video.black,black_pixel,uint,o_set);
        }

        if (j_video.find("freeze") != j_video.end()) {
            auto j_freeze = j_video.at("freeze");
            get_setting(j_freeze,"freeze",settings.video.freeze.freeze);
            get_setting(j_freeze,"diff",settings.video.freeze.diff);
            SET_VALUE_FROM_JSON(j_freeze,settings.video.freeze,pixel_diff,uint,o_set);
        }
        
        if (j_video.find("blocky") != j_video.end()) {
            auto j_blocky = j_video.at("blocky");
            get_setting(j_blocky,"blocky",settings.video.blocky.blocky);
            SET_VALUE_FROM_JSON(j_blocky,settings.video.blocky,mark_blocks,bool,o_set);
        }
    }

    if (j.find("audio") != j.end()) {
        auto j_audio = j.at("audio");

        SET_VALUE_FROM_JSON(j_audio,settings.audio,loss,float,o_set);

        if (j_audio.find("silence") != j_audio.end()) {
            auto j_silence = j_audio.at("silence");
            get_setting(j_silence,"silence",settings.audio.silence.silence);
        }
        
        if (j_audio.find("loudness") != j_audio.end()) {
            auto j_loudness = j_audio.at("loudness");
            get_setting(j_loudness,"loudness",settings.audio.loudness.loudness);
        }
        
        if (j_audio.find("adv") != j_audio.end()) {
            auto j_adv = j_audio.at("adv");
            SET_VALUE_FROM_JSON(j_adv,settings.audio.adv,adv_diff,float,o_set);
            SET_VALUE_FROM_JSON(j_adv,settings.audio.adv,adv_buf,uint,o_set);
        }
        
    }

    if (o_set) set.emit(settings);
    //send.emit(*this);
    talk();
}
