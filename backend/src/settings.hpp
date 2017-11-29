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

        Logger::level             log_level;
        vector<string>            uris;
        boost::optional<string>   multicast_address;
        boost::optional<Msg_type> msg_type;

        Initial(int argc, char** argv);
        static string usage (string prog_name);
    };

    class Settings {
    public:        
        struct Setting {
            bool cont_en;
            float cont;
            bool peak_en;
            float peak;
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

        struct Video {
            float  loss;
            Black  black;
            Freeze freeze;
            Blocky blocky;
        };

        struct Audio {
            float    loss;
            Silence  silence;
            Loudness loudness;
            Adv      adv;
        };

        Video video;
        Audio audio;
    };

    class Settings_facade : public Chatterer, public Logger {
    public:
        Settings settings;
        
        sigc::signal<void,const Settings&>   set;
    
        Settings_facade(const std::string& n) : Chatterer(n) {}
        virtual ~Settings_facade() {}

        void   init(Initial&);

        // Chatterer implementation
        string to_json_body() const;
	
        string to_string() const;
        json serialize() const;
        void deserialize(const json&);

    };

    // nlohmann json arbitrary types conversions

    void to_json(json& j, const Settings::Setting&);
    void to_json(json& j, const Settings::Black&);
    void to_json(json& j, const Settings::Freeze&);
    void to_json(json& j, const Settings::Blocky&);
    void to_json(json& j, const Settings::Loudness&);
    void to_json(json& j, const Settings::Silence&);
    void to_json(json& j, const Settings::Adv&);
    void to_json(json& j, const Settings::Video&);
    void to_json(json& j, const Settings::Audio&);
};

#endif /* SETTINGS_H */
