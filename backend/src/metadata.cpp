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

Meta_pid::Meta_pid (uint p, uint t, string tn) : pid(p), to_be_analyzed(false),
                                                 stream_type(t), stream_type_name(tn) {
    type = get_type (t);
    if (type == Type::Empty) throw Wrong_type ();
    else if (type == Type::Video) data = Video_pid ();
    else if (type == Type::Audio) data = Audio_pid ();
}

void
Ats::to_json (json& j, const Meta_pid::Video_pid& vp) {
    j = json{{"codec", vp.codec},
             {"width", vp.width},
             {"height", vp.height},
             {"aspect_ratio", {{"x", vp.aspect_ratio.first},
                               {"y", vp.aspect_ratio.second}}},
             {"interlaced", vp.interlaced},
             {"frame_rate", vp.frame_rate}};
}

void
Ats::to_json (json& j, const Meta_pid::Audio_pid& ap) {
    j = json{{"codec", ap.codec},
             {"bitrate", ap.bitrate},
             {"channels", ap.channels},
             {"sample_rate", ap.sample_rate}};
}

const Meta_pid::Audio_pid&
Meta_pid::get_audio () const {
    if(type == Meta_pid::Type::Audio) return boost::get<Audio_pid>( this->data );
    else throw Wrong_type();
}

const Meta_pid::Video_pid&
Meta_pid::get_video () const {
    if(type == Meta_pid::Type::Video) return boost::get<Video_pid>( this->data );
    else throw Wrong_type();
}

string
Meta_pid::to_string () const {
    string rval = "Pid: ";
    rval += std::to_string(pid);
    rval += "\nType: ";
    rval += (type == Meta_pid::Type::Video) ? "Video" : \
        (type == Meta_pid::Type::Audio) ? "Audio" : \
        (type == Meta_pid::Type::Subtitles) ? "Subtitles" : \
        "Unknown";
    if (type == Meta_pid::Type::Video) {
        Meta_pid::Video_pid vp = get_video();
        rval += "\nVideo info: ";
        rval += std::string("Codec: ") + vp.codec + ", ";
        rval += std::string("Width: ") + std::to_string(vp.width) + ", ";
        rval += std::string("Height: ") + std::to_string(vp.height) + ", ";
        rval += std::string("Aspect ratio: ") + std::to_string(vp.aspect_ratio.first) + \
            ":" + std::to_string(vp.aspect_ratio.second) + ", ";
        rval += std::string("Interlaced: ") + vp.interlaced + ", ";
        rval += std::string("Frame rate: ") + std::to_string(vp.frame_rate);
    }
    else if (type == Meta_pid::Type::Audio) {
        Meta_pid::Audio_pid ap = get_audio();
        rval += "\nAudio info: ";
        rval += std::string("Codec: ") + ap.codec + ", ";
        rval += std::string("Bitrate: ") + ap.bitrate + ", ";
        rval += std::string("Channels: ") + std::to_string(ap.channels) + ", ";
        rval += std::string("Sample rate: ") + std::to_string(ap.sample_rate);
    }
    rval += "\nStream type: ";
    rval += std::to_string(stream_type);
    rval += "\nStream type name: ";
    rval += stream_type_name;
    rval += "\nTo be analyzed: ";
    rval += Ats::to_string(to_be_analyzed);
    return rval;
}

void
Ats::to_json (json& j, const Meta_pid& p) {
    json j_description = json::object();
    if (p.type == Meta_pid::Type::Video)
        j_description = p.get_video();
    else if (p.type == Meta_pid::Type::Audio)
        j_description = p.get_audio();

    j = {{"pid", p.pid},
         {"to_be_analyzed", p.to_be_analyzed},
         {"type", (p.type == Meta_pid::Type::Video ? "video" :
                   p.type == Meta_pid::Type::Audio ? "audio" :
                   p.type == Meta_pid::Type::Subtitles ? "subtitles" :
                   p.type == Meta_pid::Type::Teletext ? "teletext" :
                   "empty")},
         {"stream_type", p.stream_type},
         {"stream_type_name", p.stream_type_name},
         {"description", j_description}};
}

// --------- Meta_channel  ---------------

Meta_pid*
Meta_channel::find_pid (uint pid) {
    for (Meta_pid& p : pids) {
        if (p.pid == pid) return &p;
    }
    return nullptr;
}

const Meta_pid*
Meta_channel::find_pid (uint pid) const {
    for (const Meta_pid& p : pids) {
        if (p.pid == pid) return &p;
    }
    return nullptr;
}

string
Meta_channel::to_string () const {
    string rval = "PMT PID: ";
    rval += std::to_string(number);
    rval += "\nService name: ";
    rval += service_name;
    rval += "\nProvider name: ";
    rval += provider_name;
    rval += "\nPids:";
    for (auto it=pids.begin(); it!=pids.end(); ++it) {
        rval += "\n";
        if ( it == pids.begin() )
            rval += "***\n";
        rval += it->to_string();
        rval += "\n***";
    }
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

void
Ats::to_json (json& j, const Meta_channel& c) {
    json j_pids(c.pids);
    j = json{{"number", c.number},
             {"service_name", c.service_name},
             {"provider_name", c.provider_name},
             {"pids", j_pids}};
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
    rval += "\n";
    string channels_str = "\tChannels:";
    for (auto it = channels.begin(); it != channels.end(); ++it) {
            channels_str += "\n";
            if ( it == channels.begin())
                channels_str += "-------------------------------------------\n";
            channels_str += it->to_string();
            channels_str += "\n-------------------------------------------";
    }
    rval += Ats::add_indent(channels_str, 1);
    return rval;
}

void
Ats::to_json(json& j, const Metadata& m) {
    json j_channels(m.channels);
    j = json{{"stream", m.stream},
             {"uri",m.uri},
             {"channels", j_channels}};
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
