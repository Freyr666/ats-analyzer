#ifndef AUDIODATA_H
#define AUDIODATA_H

#include <gstreamermm.h>
#include <vector>
#include "chatterer.hpp"

namespace Ats {

    class Audio_data : public Chatterer_data {
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
        
        Audio_data () : Chatterer_data("audio_data") {}

        void parse_data_msg(uint, uint, uint,
                            int64_t, Glib::RefPtr<Gst::Buffer>,
                            int64_t, Glib::RefPtr<Gst::Buffer>) {}
    };

}

#endif /* AUDIODATA_H */
