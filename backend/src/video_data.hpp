#ifndef VIDEODATA_H
#define VIDEODATA_H

#include <gstreamermm.h>
#include <vector>
#include "chatterer.hpp"

namespace Ats {

    class Video_data : public Chatterer_data {
    public:
        
        struct params {
            float   frozen_pix;
            float   black_pix;
            float   blocks;
            float   avg_bright;
            float   avg_diff;
            int64_t time; 
        };

        enum parameter { BLACK, LUMA, FREEZE, DIFF, BLOCKY, PARAM_NUMBER };

        struct error_flags {
            bool cont;
            bool peak;
            int64_t time;
        };

        struct msg {
            uint stream;
            uint channel;
            uint pid;
            std::vector <params>      parameters;
            std::vector <error_flags> errors [PARAM_NUMBER];
        };
        
        Video_data () : Chatterer_data("video_data") {}

        void parse_data_msg(uint, uint, uint,
                            int64_t, Glib::RefPtr<Gst::Buffer>,
                            int64_t, Glib::RefPtr<Gst::Buffer>);
    };

    void to_json(json& j, const Video_data::params&);
    void to_json(json& j, const Video_data::error_flags&);
    void to_json(json& j, const Video_data::msg&);
}

#endif /* VIDEODATA_H */
