#ifndef VIDEODATA_H
#define VIDEODATA_H

#include "chatterer.hpp"

namespace Ats {

    class Video_data : public Chatterer {
    public:
        Video_data () : Chatterer("video_data") {}

        bool parse () {}
    };
}

#endif /* VIDEODATA_H */
