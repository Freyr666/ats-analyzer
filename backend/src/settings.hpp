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
        struct Channel_settings {

            struct Error_overlay {
                uint error_color = 0;
                bool enabled = true;

                string to_string() const;
            };

            struct Channel_name {
                bool enabled = true;
                int font_size = 14; //maybe should be relative to normal size
                string fmt = "$INPUT_TYPE, $INPUT_NAME, $CH_NAME";

                string to_string() const;
            };

            struct Audio_meter {
                enum class Audio_meter_pos {Left, Right};

                bool enabled = true;
                Audio_meter_pos position = Audio_meter_pos::Right;

                string to_string() const;
            };

            struct Status_bar {
                enum class Status_bar_pos {Top_left, Top_right,
                                           Left, Right,
                                           Bottom_left, Bottom_right};

                Status_bar_pos position = Status_bar_pos::Top_left;
                bool enabled = true;
                bool aspect = true;
                bool subtitles = true;
                bool teletext = true;
                bool eit = true;
                bool qos = true;
                bool scte35 = true;

                string to_string() const;
            };

            bool show_border = false;
            uint border_color = 0;
            bool show_aspect_border = false;
            uint aspect_border_color = 0;
            Error_overlay error_overlay;
            Channel_name channel_name;
            Audio_meter audio_meter;
            Status_bar status_bar;

            string to_string() const;
        };

        struct Qoe_settings {

            struct Setting {
                bool cont_en;
                float cont;
                bool peak_en;
                float peak;
            };

            struct Loss {
                float vloss;
                float aloss;
            };

            struct Black {
                Setting black;
                Setting luma;
                float time;
                uint black_pixel;
            };

            struct Freeze {
                Setting freeze;
                Setting diff;
                float time;
                uint pixel_diff;
            };

            struct Blocky {
                Setting blocky;
                float time;
                bool mark_blocks;
            };

            struct Silence {
                Setting silence;
                float time;
            };

            struct Loudness {
                Setting loudness;
                float time;
            };

            struct Adv {
                float adv_diff;
                uint adv_buf;
            };

            Loss loss;
            Black black;
            Freeze freeze;
            Blocky blocky;
            Silence silence;
            Loudness loudness;
            Adv adv;

            // FIXME add default constructors?

            string to_string() const;
        };

        /* ------- Qoe analysis settings -------- */
        Qoe_settings qoe_settings;

        /* ------- Mosaic settings -------------- */
        Channel_settings channel_settings;

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
    void to_json(json& j, const Settings::Channel_settings::Error_overlay&);
    void to_json(json& j, const Settings::Channel_settings::Channel_name&);
    void to_json(json& j, const Settings::Channel_settings::Audio_meter&);
    void to_json(json& j, const Settings::Channel_settings::Status_bar&);
    void to_json(json& j, const Settings::Channel_settings&);

    void to_json(json& j, const Settings::Qoe_settings::Setting&);
    void to_json(json& j, const Settings::Qoe_settings::Loss&);
    void to_json(json& j, const Settings::Qoe_settings::Black&);
    void to_json(json& j, const Settings::Qoe_settings::Freeze&);
    void to_json(json& j, const Settings::Qoe_settings::Blocky&);
    void to_json(json& j, const Settings::Qoe_settings::Loudness&);
    void to_json(json& j, const Settings::Qoe_settings::Silence&);
    void to_json(json& j, const Settings::Qoe_settings::Adv&);
    void to_json(json& j, const Settings::Qoe_settings&);
};

#endif /* SETTINGS_H */
