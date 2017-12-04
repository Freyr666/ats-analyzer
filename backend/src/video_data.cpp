#include "video_data.hpp"

using namespace Ats;

#include <iostream>

void
Video_data::parse_data_msg(uint stream, uint channel, uint pid,
                           int64_t ds, Glib::RefPtr<Gst::Buffer> d,
                           int64_t es, Glib::RefPtr<Gst::Buffer> e) {
    auto mmap = Glib::RefPtr<Gst::MapInfo>(new Gst::MapInfo());
    msg _msg;
    
    _msg.stream = stream;
    _msg.channel = channel;
    _msg.pid = pid;
    /* data */
    d->map(mmap, Gst::MapFlags::MAP_READ);
    params* data_buf = (params*) mmap->get_data();
    std::vector<params> data (data_buf, data_buf + ds);
    _msg.parameters = std::move(data);
    d->unmap(mmap);
    /* errors */
    e->map(mmap, Gst::MapFlags::MAP_READ);
    error_flags* errors_buf = (error_flags*) mmap->get_data();
    for (int p = 0; p < PARAM_NUMBER; p++) {
        std::vector<error_flags> errors (errors_buf + (p * es), errors_buf + (p * es) + es);
        _msg.errors[p] = std::move(errors);
    }
    e->unmap(mmap);

    json j = _msg;
    
    data_send(std::move(j));
}

void
Ats::to_json(json& j, const Ats::Video_data::params& p) {
    j = {{"frozen_pix", p.frozen_pix},
         {"black_pix", p.black_pix},
         {"blocks", p.blocks},
         {"avg_bright", p.avg_bright},
         {"avg_diff", p.avg_diff},
         {"time", p.time}};
}

void
Ats::to_json(json& j, const Ats::Video_data::error_flags& e) {
    j = {{"cont", e.cont},
         {"peak", e.peak},
         {"time", e.time}};
}

void
Ats::to_json(json& j, const Ats::Video_data::msg& m) {
    j = {{"stream", m.stream},
         {"channel", m.channel},
         {"pid", m.pid},
         {"parameters", m.parameters},
         {"errors", json{{"black", m.errors[Video_data::BLACK]},
                         {"luma", m.errors[Video_data::LUMA]},
                         {"freeze", m.errors[Video_data::FREEZE]},
                         {"diff", m.errors[Video_data::DIFF]},
                         {"blocky", m.errors[Video_data::BLOCKY]}}}};
}
