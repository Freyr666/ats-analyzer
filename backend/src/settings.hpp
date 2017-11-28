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

    class Settings : public Chatterer, public Logger {

    public:

        struct QoE {
        
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

        QoE qoe;
        
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

    void to_json(json& j, const Settings::QoE::Setting&);
    void to_json(json& j, const Settings::QoE::Black&);
    void to_json(json& j, const Settings::QoE::Freeze&);
    void to_json(json& j, const Settings::QoE::Blocky&);
    void to_json(json& j, const Settings::QoE::Loudness&);
    void to_json(json& j, const Settings::QoE::Silence&);
    void to_json(json& j, const Settings::QoE::Adv&);
    void to_json(json& j, const Settings::QoE::Video&);
    void to_json(json& j, const Settings::QoE::Audio&);
};

#endif /* SETTINGS_H */
