#include "metadata.hpp"

#include <algorithm>
#include <gst/mpegts/gstmpegtssection.h>

using namespace std;
using namespace Ats;

// --------- Position -------------------

bool
Position::operator== (const Position& a) {
    return ((a.x == x) && (a.y == y) &&
            (a.width == width) && (a.height == height));
}

bool
Position::operator!= (const Position& a) {
    return !(*this == a);
}

bool
Position::is_overlap (const Position& a) {
    return ((a.x < (width + x)) && ((a.width + a.x) > x) &&
            (a.y < (height + y)) && ((a.height + a.y) > y));
}

// --------- Meta_pid -------------------

Meta_pid::Type
Meta_pid::get_type (uint stream_type) {
    switch (stream_type) {
    case GST_MPEGTS_STREAM_TYPE_VIDEO_MPEG1:
    case GST_MPEGTS_STREAM_TYPE_VIDEO_MPEG2:
    case GST_MPEGTS_STREAM_TYPE_VIDEO_MPEG4:
    case GST_MPEGTS_STREAM_TYPE_VIDEO_H264:
    case GST_MPEGTS_STREAM_TYPE_VIDEO_RVC:
    case GST_MPEGTS_STREAM_TYPE_VIDEO_H264_SVC_SUB_BITSTREAM:
    case GST_MPEGTS_STREAM_TYPE_VIDEO_H264_MVC_SUB_BITSTREAM:
    case GST_MPEGTS_STREAM_TYPE_VIDEO_JP2K:
    case GST_MPEGTS_STREAM_TYPE_VIDEO_MPEG2_STEREO_ADDITIONAL_VIEW:
    case GST_MPEGTS_STREAM_TYPE_VIDEO_H264_STEREO_ADDITIONAL_VIEW:
    case GST_MPEGTS_STREAM_TYPE_VIDEO_HEVC:
        return Type::Video;
    case GST_MPEGTS_STREAM_TYPE_AUDIO_MPEG1:
    case GST_MPEGTS_STREAM_TYPE_AUDIO_MPEG2:
    case GST_MPEGTS_STREAM_TYPE_AUDIO_AAC_ADTS:
    case GST_MPEGTS_STREAM_TYPE_AUDIO_AAC_LATM:
    case GST_MPEGTS_STREAM_TYPE_AUDIO_AAC_CLEAN:
        return Type::Audio;
    default:
        return Type::Empty;
    }
}

Meta_pid::Meta_pid (uint p, uint t, string tn) : pid(p), to_be_analyzed(true),
                                                 stream_type(t), stream_type_name(tn) {
    type = get_type (t);
    if (type == Type::Empty) throw Wrong_type ();
}

Meta_pid::Audio_pid&
Meta_pid::get_audio () {
    if(type == Meta_pid::Type::Audio)
        return audio;
    else
        throw Wrong_type();
}

Meta_pid::Video_pid&
Meta_pid::get_video () {
    if(type == Meta_pid::Type::Video)
        return video;
    else
        throw Wrong_type();
}

string
Meta_pid::to_string () const {
    string rval = "Pid: ";
    rval += std::to_string(pid);
    rval += " Type: ";
    rval += std::to_string(stream_type);
    rval += " Codec: ";
    rval += stream_type_name;
    return rval;
}

// --------- Meta_channel  ---------------

Meta_pid*
Meta_channel::find_pid (uint pid) {
    for (Meta_pid& p : pids) {
	if (p.pid == pid) return &p;
    }
    return nullptr;
}

string
Meta_channel::to_string () const {
    string rval = "Channel: ";
    rval += std::to_string(number);
    rval += " Service name: ";
    rval += service_name;
    rval += " Provider name: ";
    rval += provider_name;
    rval += " Pids: ";
    for_each (pids.begin(), pids.end(), [&rval](const Meta_pid& p) {
	    rval += "(";
	    rval += p.to_string();
	    rval += ");";
	});
    return rval;
}

bool
Meta_channel::to_be_analyzed () const {
    if (pids.empty()) return false;
    
    auto result = find_if (pids.begin(), pids.end(), [](const Meta_pid& p){
	    return p.to_be_analyzed;
	});
    return result != pids.end();
}

// ---------- Metadata ---------------------


Meta_pid*
Metadata::find_pid (uint chan, uint pid) {
    for (Meta_channel& c : channels) {
	if (c.number == chan)
	    for (Meta_pid& p : c.pids) {
		if (p.pid == pid) return &p;
	    }
    }
    return nullptr;
}

const Meta_pid*
Metadata::find_pid (uint chan, uint pid) const {
    for (const Meta_channel& c : channels) {
	if (c.number == chan)
	    for (const Meta_pid& p : c.pids) {
		if (p.pid == pid) return &p;
	    }
    }
    return nullptr;
}

Meta_pid*
Metadata::find_pid (uint pid) {
    for (Meta_channel& c : channels) {
	for (Meta_pid& p : c.pids) {
	    if (p.pid == pid) return &p;
	}
    }
    return nullptr;
}

const Meta_pid*
Metadata::find_pid (uint pid) const {
    for (const Meta_channel& c : channels) {
	for (const Meta_pid& p : c.pids) {
	    if (p.pid == pid) return &p;
	}
    }
    return nullptr;
}

Meta_channel*
Metadata::find_channel (uint chan) {
    for (Meta_channel& c : channels) {
	if (c.number == chan) return &c;
    }
    return nullptr;
}

const Meta_channel*
Metadata::find_channel (uint chan) const {
    for (const Meta_channel& c : channels) {
	if (c.number == chan) return &c;
    }
    return nullptr;
}

string
Metadata::to_string () const {
    string rval = "Stream: ";
    rval += std::to_string(stream);
    rval += " Channels: ";
    for_each (channels.begin(), channels.end(), [&rval](const Meta_channel& c) {
	    rval += "[";
	    rval += c.to_string();
	    rval += "];";
	});
    return rval;
}

bool
Metadata::to_be_analyzed () const {
    if (channels.empty()) return false;
    
    auto rval = find_if (channels.begin(),channels.end(),[](const Meta_channel& c){
            return c.to_be_analyzed();
        });
    return rval != channels.end();
}

void
Metadata::for_analyzable (std::function<void(const Meta_channel&)> fun) const {
    for_each(channels.begin(),channels.end(),[&fun](const Meta_channel& c){
            if (c.to_be_analyzed()) fun(c);
        });
}

bool
Metadata::validate_grid (uint width, uint height) const {

    bool failure = false;

    for (auto channel = channels.begin(); channel != channels.end(); channel++) {

        for (auto pid = (*channel).pids.begin(); pid != (*channel).pids.end(); pid++) {

            auto cur_pid = *pid;

            /* check current program pids */
            auto rval_inner = find_if ((pid+1), (*channel).pids.end(),
                                       [&cur_pid](const Meta_pid& p) {
                                           if (cur_pid.position != p.position) {
                                               if ((cur_pid.type == p.type) &&
                                                   (cur_pid.type == Meta_pid::Type::Video))
                                                   return false;
                                               else
                                                   return cur_pid.position.is_overlap(p.position);
                                           }
                                           else return true;
                                       });

            /* compare with other programs pids */
            auto rval_outer = find_if ((channel+1),channels.end(),[this,&cur_pid](const Meta_channel& c){
                    auto rval = find_if(c.pids.begin(), c.pids.end(),
                                        [&cur_pid](const Meta_pid &p) {
                                            return cur_pid.position.is_overlap(p.position);
                                        });

                    return rval != c.pids.end();
                });

            /* decide if there is a problem in a grid */
            if (((cur_pid.position.width > width) || (cur_pid.position.x < 0)) ||
                ((cur_pid.position.height > height) || (cur_pid.position.y < 0)) ||
                (rval_inner != (*channel).pids.end()) ||
                (rval_outer != channels.end())) {
                failure = true;
                break;
            }

        }

        if (failure) break;
    }

    return !failure;
}
