#ifndef OPTIONS_H
#define OPTIONS_H

#include <vector>
#include <gstreamermm.h>

#include "metadata.hpp"
#include "probe.hpp"
#include "address.hpp"

using namespace std;

namespace Ats {

    struct Options {

        struct Channel_settings {

            struct Error_overlay {
                string error_color = "";
                float blink_speed = 1; // in Hz
                bool enabled = true;
            };

            struct Channel_name {
                enum class Channel_name_pos {Off, Left, Right, Center};

                Channel_name_pos fs_pos = Channel_name_pos::Center;
                int font_size = 14; //maybe should be relative to normal size
                bool with_input_name = false;
                bool input_name_fmt = "$INPUT_TYPE, $INPUT_NAME, $CH_NAME";
                bool fullscreen = false;
            };

            struct Audio_meter {
                enum class Audio_meter_pos {Off, Left, Right};

                Audio_meter_pos audio_meter_pos = Audio_meter_display::Right;
                bool overlay = true;
                int width = 8; // tbd. Units? Pixels?
                int height = 100; // tbd. In percents?

                string peak_color = "";
                string high_color = "";
                string mid_color = "";
                string low_color = "";
                string background_color = "";
            };

            struct Status_bar {
                enum class Status_bar_pos {Off, Top_left, Top_right,
                                           Left, Right,
                                           Bottom_left, Bottom_right};

                Status_bar_pos status_bar_pos = Status_bar_pos::Top_left;
                bool aspect = true;
                bool subtitles = true;
                bool teletext = true;
                bool eit = true;
                bool qos = true;
                bool scte35 = true;
            };

            bool show_border = false;
            string border_color = "";
            bool show_aspect_border = false;
            string aspect_border_color = "";
            Error_overlay error_overlay;
            Channel_name channel_name;
            Audio_meter audio_meter;
            Status_bar status_bar;
        };

        struct Qoe_settings {
            /* loss */
            float vloss;
            float aloss;
            /* black frame */
            bool black_cont_en;
            float black_cont;
            bool black_peak_en;
            float black_peak;
            bool luma_cont_en;
            float luma_cont;
            bool luma_peak_en;
            float luma_peak;
            float black_time;
            int black_pixel;
            /* freeze */
            bool freeze_cont_en;
            float freeze_cont;
            bool freeze_peak_en;
            float freeze_peak;
            bool diff_cont_en;
            float diff_cont;
            bool diff_peak_en;
            float diff_peak;
            float freeze_time;
            int pixel_diff;
            /* blockiness */
            bool blocky_cont_en;
            float blocky_cont;
            bool blocky_peak_en;
            float blocky_peak;
            float blocky_time;
            bool mark_blocks;
            /* silence */
            bool silence_cont_en;
            float silence_cont;
            bool silence_peak_en;
            float silence_peak;
            float silence_time;
            /* loudness */
            bool loudness_cont_en;
            float loudness_cont;
            bool loudness_peak_en;
            float loudness_peak;
            float loudness_time;
            /* adv loudness */
            float adv_diff;
            int adv_buf;
        };

        /* ------- Prog list -------------------- */
        vector<Metadata> data;

        /* ------- Qoe analysis settings -------- */
        Qoe_settings qoe_settings;

        /* ------- Mosaic settings -------------- */
        pair<int,int> resolution;
        string background_color;
        Channel_settings channel_settings;

        /* ------- Output stream settings ------- */
        Address output_sink;

        sigc::signal<void,const Options&> updated;
    
        Options() {}
        ~Options() {}

        void   connect(Probe& p) { p.updated.connect(
                sigc::mem_fun(this, &Options::set_data));
        }

        void   set_data(const Metadata&);
        string to_string() const;
        string to_json()   const;
        void   of_json(const string&);

        void operator=(const Metadata& m) { set_data(m); }
        void operator=(const string& js) { of_json(js); }
    };

};

#endif /* OPTIONS_H */
