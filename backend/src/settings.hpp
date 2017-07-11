#ifndef SETTINGS_H
#define SETTINGS_H

#include <string>
#include <gstreamermm.h>
#include <boost/optional.hpp>

#include "msgtype.hpp"
#include "chatterer.hpp"
#include "errexpn.hpp"

using namespace std;

namespace Ats {

    using json = nlohmann::json;

    struct Initial {
        struct Wrong_option : public Error_expn {
            Wrong_option() : Error_expn() {}
            Wrong_option(string s) : Error_expn(s) {}
        };
	
        vector<string>            uris;
        boost::optional<string>   multicast_address;
        boost::optional<Msg_type> msg_type;

        Initial(int argc, char** argv);
        static string usage (string prog_name);
    };

    class Settings : public Chatterer, public Logger {

    public:

        struct Qoe_settings {
            /* FIXME add correct defaults */
            /* loss */
            float vloss = 2.;
            float aloss = 2.;
            /* black frame */
            bool black_cont_en = true;
            float black_cont = 90.;
            bool black_peak_en = true;
            float black_peak = 100.;
            bool luma_cont_en = true;
            float luma_cont = 20.;
            bool luma_peak_en = false;
            float luma_peak = 16.;
            float black_time = 10.;
            uint black_pixel = 16;
            /* freeze */
            bool freeze_cont_en = true;
            float freeze_cont = 90.;
            bool freeze_peak_en = true;
            float freeze_peak = 100.;
            bool diff_cont_en = true;
            float diff_cont = .1;
            bool diff_peak_en = false;
            float diff_peak = .02;
            float freeze_time = 10.;
            uint pixel_diff = 0;
            /* blockiness */
            bool blocky_cont_en = true;
            float blocky_cont = 4.;
            bool blocky_peak_en = true;
            float blocky_peak = 7.;
            float blocky_time = 3.;
            bool mark_blocks = false;
            /* silence */
            bool silence_cont_en = true;
            float silence_cont = -35.;
            bool silence_peak_en = false;
            float silence_peak = -45.;
            float silence_time = 10.;
            /* loudness */
            bool loudness_cont_en = true;
            float loudness_cont = -22.;
            bool loudness_peak_en = true;
            float loudness_peak = -15.;
            float loudness_time = 2.;
            /* adv loudness */
            float adv_diff = 1.5;
            uint adv_buf = 2 * 60 * 60;

            string to_string() const;
        };

        /* ------- Qoe analysis settings -------- */
        Qoe_settings qoe_settings;

        sigc::signal<void,const Settings&>   set;
    
        Settings(const std::string& n) : Chatterer(n) {}
        virtual ~Settings() {}

        void   init(Initial&);

        // Chatterer implementation
        string to_json_body() const;
	
        string to_string() const;
        json serialize() const;
        void deserialize(const json&);

    };

    // nlohmann json arbitrary types conversions
    void to_json(json& j, const Settings::Qoe_settings&);
};

#endif /* SETTINGS_H */
