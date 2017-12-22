#include <gstreamermm/tee.h>

#include "root.hpp"

#include <iostream>

using namespace Ats;
using namespace std;

Root::Root (const Glib::RefPtr<Gst::Bin> bin,
            const Metadata& m, const Settings& s,
            std::shared_ptr<Video_data> vd, std::shared_ptr<Audio_data> ad) : _settings(s) {
    uint stream = m.stream;
    
    _bin   = bin;
    _src   = Gst::ElementFactory::create_element("udpsrc");
    auto parse = Gst::ElementFactory::create_element("tsparse");
    _tee   = Gst::Tee::create();
    _video_sender = vd;
    _audio_sender = ad;

    _src->set_property("uri", m.uri);
    _src->set_property("buffer-size", 2147483647);

    _bin->add(_src)->add(parse)->add(_tee);

    _src->link(parse)->link(_tee);

    m.for_analyzable ([this,stream](const Meta_channel& c) { build_cb(stream,c); });
}

Root::~Root() {
    _bin.reset();
}

unique_ptr<Root>
Root::create (const Glib::RefPtr<Gst::Bin> bin,
              const Metadata& m, const Settings& s,
              std::shared_ptr<Video_data> vd, std::shared_ptr<Audio_data> ad) {
    if (! m.to_be_analyzed()) return unique_ptr<Root>(nullptr);
    else return unique_ptr<Root>(new Root(bin, m, s, vd, ad));
}

void
Root::apply (const Settings& s) {
    _settings = s;
    for (auto& branch : _branches) {
        branch->apply(s);
    }
}

void
Root::build_cb (const uint stream, const Meta_channel& c) {
    string demux_name = "demux_";
    demux_name += std::to_string(stream);
    demux_name += "_";
    demux_name += std::to_string(c.number);
    //cout << "Demux name: " << demux_name << endl;

    auto queue = Gst::ElementFactory::create_element("queue");
    auto _demux = Gst::ElementFactory::create_element("tsdemux",demux_name);
    
    _demux->set_property("program-number", c.number);
    queue->set_property("max-size-buffers", 200000);
    queue->set_property("max-size-bytes", 429496729);
    
    auto sinkpad = queue->get_static_pad("sink"); 
    auto srcpad = _tee->get_request_pad("src_%u");

    _bin->add(queue)->add(_demux);
    queue->link(_demux);

    srcpad->link(sinkpad);
    // GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN(_bin->gobj()), GST_DEBUG_GRAPH_SHOW_ALL, "pipeline");
    _demux->signal_pad_added().connect([this, stream, c](const Glib::RefPtr<Gst::Pad>& p)
                                       { build_branch(stream, c, p); });
    //_demux->signal_pad_removed().connect([this, stream, num](const Glib::RefPtr<Gst::Pad>& p)
    //				      { destroy_branch(stream, num, p); });
}

void
Root::build_branch (const uint stream,
		    const Meta_channel c,
		    const Glib::RefPtr<Gst::Pad>& p) {
    auto num   = c.number;
    auto pname = p->get_name();
    auto pcaps = p->get_current_caps()->get_structure(0).get_name();
    vector<Glib::ustring> name_toks = Glib::Regex::split_simple("_", pname);
    vector<Glib::ustring> caps_toks = Glib::Regex::split_simple("/", pcaps);

    auto& type = caps_toks[0];	
		    
    if (type != "video" && type != "audio") return;
    
    auto pid  = strtoul(name_toks[2].data(), NULL, 16);

    if (! c.find_pid(pid)->to_be_analyzed) return;

    auto branch = Branch::create(type, stream, num, pid, _video_sender, _audio_sender);

    if (branch == nullptr) return;

    branch->add_to_pipe(_bin);
    branch->plug(p);
	
    branch->signal_pad_added().connect([this](std::shared_ptr<Pad> p){
            std::scoped_lock lock{_mutex_pad};
            _pad_added.emit(p);
        });
    
    if (branch->type() == Branch::Type::Audio) {
        Audio_branch* b = dynamic_cast<Audio_branch*>(branch.get());
        b->signal_audio_pad_added().connect([this](std::shared_ptr<Pad> p){
                std::scoped_lock lock{_mutex_audio_pad};
                _audio_pad_added.emit(p);
            });
    }

    branch->apply(_settings);
    _branches.push_back(std::move(branch));
    _bin->set_state(Gst::State::STATE_PLAYING);
}

void
Root::destroy_branch (const uint stream,
                      const uint num,
                      const Glib::RefPtr<Gst::Pad>& p) {
	
    auto pname = p->get_name();
    auto pcaps = p->get_current_caps()->get_structure(0).get_name();

    vector<Glib::ustring> name_toks = Glib::Regex::split_simple("_", pname);
    vector<Glib::ustring> caps_toks = Glib::Regex::split_simple("/", pcaps);

    auto& type = caps_toks[0];	
		    
    if (type != "video" && type != "audio") return;
    
    auto pid  = strtoul(name_toks[2].data(), NULL, 16);

    for (auto it = _branches.begin(); it != _branches.end(); it++) {
        if (*(*it) == make_tuple(stream, num, pid)) {
            _branches.erase(it);
            break;
        }
    }   
}
