#include "metadata.hpp"

#include <algorithm>
#include <gst/mpegts/gstmpegtssection.h>

using namespace std;
using namespace Ats;

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
    // rval += std::to_string(pid);
    // rval += " Type: ";
    // rval += std::to_string(type);
    // rval += " Codec: ";
    // rval += codec;
    // return rval;
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

    auto do_rects_overlap = [](Meta_pid::Position& a, Meta_pid::Position& b) {
        return ((a.x <= (b.width - b.x)) and
                ((a.width - a.x) >= b.x) and
                (a.y <= (b.height - b.y)) and
                ((a.height - a.y) > b.y));
    }
    /*
      logic:
      * first check if pid positions are equal or, if not, if they do not cross
      * then check if unique pid positions do not cross with positions of pids in other progs
      */

    for (channel = channels.begin(); channel != channels.end(); channel++) {

        bool failure = false;

        for (pid = *channel.pids.begin(); pid != *channel.pid.end(); pid++) {
            if (((*pid.position.first > width) || (*pid.position.first < 0)) &&
                ((*pid.position.second > height) || (*pid.position.second < 0))
                (for_each ((pid+1), *channel.pids.end(), [&(*pid)](Meta_pid& p) {
                        return do_rects_overlap(p.position, pid.position);
                    }))) {
                failure = true;
                break;
            }
        }

        auto rval = find_if_not ((channel+1), channels.end(), compare);
    }

    return rval == channels.end();
}
