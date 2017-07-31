#include "settings.hpp"
#include "validate.hpp"

using namespace std;
using namespace Ats;

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

/* ---------------- End of QoE settings -------------- */

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

    if (o_set) set.emit(*this);
}
