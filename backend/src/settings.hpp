#ifndef SETTINGS_H
#define SETTINGS_H

#include <string>
#include <gstreamermm.h>
#include <boost/optional.hpp>

#include "msgtype.hpp"
#include "chatterer.hpp"
#include "probe.hpp"

using namespace std;

namespace Ats {

    struct Initial {
	vector<string>            uris;
	boost::optional<string>   multicast_address;
	boost::optional<Msg_type> msg_type;

	Initial(int argc, char** argv);
    };

    class Settings : public Chatterer {

    public:
        struct Channel_settings {

            struct Error_overlay {
                string error_color = "";
                float blink_speed = 1; // in Hz
                bool enabled = true;

                string to_json() const;
                void   of_json(const string&);
            };

            struct Channel_name {
                enum class Channel_name_pos {Off, Left, Right, Center};

                Channel_name_pos position = Channel_name_pos::Center;
                int font_size = 14; //maybe should be relative to normal size
                string fmt = "$INPUT_TYPE, $INPUT_NAME, $CH_NAME";

                string to_json() const;
                void   of_json(const string&);
            };

            struct Audio_meter {
                enum class Audio_meter_pos {Off, Left, Right};

                Audio_meter_pos position = Audio_meter_pos::Right;
                int width = 8; // tbd. Units? Pixels?
                int height = 100; // tbd. In percents?

                string peak_color = "";
                string high_color = "";
                string mid_color = "";
                string low_color = "";
                string background_color = "";

                string to_json() const;
                void   of_json(const string&);
            };

            struct Status_bar {
                enum class Status_bar_pos {Off, Top_left, Top_right,
                                           Left, Right,
                                           Bottom_left, Bottom_right};

                Status_bar_pos position = Status_bar_pos::Top_left;
                bool aspect = true;
                bool subtitles = true;
                bool teletext = true;
                bool eit = true;
                bool qos = true;
                bool scte35 = true;

                string to_json() const;
                void   of_json(const string&);
            };

            bool show_border = false;
            string border_color = "";
            bool show_aspect_border = false;
            string aspect_border_color = "";
            Error_overlay error_overlay;
            Channel_name channel_name;
            Audio_meter audio_meter;
            Status_bar status_bar;

            string to_json() const;
            void   of_json(const string&);
        };

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
            int black_pixel = 16;
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
            int pixel_diff = 0;
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
            int adv_buf = 2 * 60 * 60;

            string to_json() const;
            void   of_json(const string&);
        };

        struct Output_sink {
            string address = "239.0.0.1";
            int port = 1234;
            bool enabled = true;

            string to_json() const;
            void   of_json(const string&);
        };

        /* ------- Qoe analysis settings -------- */
        Qoe_settings qoe_settings;

        /* ------- Mosaic settings -------------- */
        Channel_settings channel_settings;

        /* ------- Output sink settings ------- */
        Output_sink output_sink_settings;

        sigc::signal<void,const Settings&>   set;
    
        Settings() {}
        virtual ~Settings() {}

	void   init(Initial&);

        // Chatter implementation
        string to_string() const;	
        string to_json()   const;
        void   of_json(const string&);
        string to_msgpack()   const;
        void   of_msgpack(const string&);
    };
};

#endif /* SETTINGS_H */
