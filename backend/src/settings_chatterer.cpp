#include "settings.hpp"
#include "validate.hpp"

using namespace std;
using namespace Ats;

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
