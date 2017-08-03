#include <gstreamermm/tee.h>

#include "root.hpp"

#include <iostream>

using namespace Ats;
using namespace std;

Root::Root (const Glib::RefPtr<Gst::Bin> bin, const Metadata& m) {
    uint stream = m.stream;
    
    _bin   = bin;
    auto src   = Gst::ElementFactory::create_element("udpsrc");
    auto parse = Gst::ElementFactory::create_element("tsparse");
    _tee   = Gst::Tee::create();

    src->set_property("uri", m.uri);
    src->set_property("buffer-size", 2147483647);

    _bin->add(src)->add(parse)->add(_tee);

    src->link(parse)->link(_tee);

    m.for_analyzable ([this,stream](const Meta_channel& c) { build_cb(stream,c); });
}

unique_ptr<Root>
Root::create (const Glib::RefPtr<Gst::Bin> bin, const Metadata& m) {
    if (! m.to_be_analyzed()) return unique_ptr<Root>(nullptr);
    else return unique_ptr<Root>(new Root(bin, m));
}

void
Root::build_cb (const uint stream, const Meta_channel& c) {
    uint num = c.number;

    string demux_name = "demux_";
    demux_name += std::to_string(stream);
    demux_name += "_";
    demux_name += std::to_string(num);
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
    _demux->signal_pad_added().connect([this, stream, num](const Glib::RefPtr<Gst::Pad>& p)
				      { build_branch(stream, num, p); });
    //_demux->signal_pad_removed().connect([this, stream, num](const Glib::RefPtr<Gst::Pad>& p)
//				      { destroy_branch(stream, num, p); });
}

void
Root::build_branch (const uint stream,
		    const uint num,
		    const Glib::RefPtr<Gst::Pad>& p) {
    auto pname = p->get_name();
    auto pcaps = p->get_current_caps()->get_structure(0).get_name();
    vector<Glib::ustring> name_toks = Glib::Regex::split_simple("_", pname);
    vector<Glib::ustring> caps_toks = Glib::Regex::split_simple("/", pcaps);

    auto& type = caps_toks[0];	
		    
    if (type != "video" && type != "audio") return;
    
    auto pid  = strtoul(name_toks[2].data(), NULL, 16);

    auto branch = Branch::create(type, stream, num, pid);

    if (branch == nullptr) return;

    branch->add_to_pipe(_bin);
    branch->plug(p);
	
    branch->signal_pad_added().connect([this](std::shared_ptr<Pad> p){ _pad_added.emit(p); });
    if (branch->type() == Branch::Type::Audio) {
        Audio_branch* b = dynamic_cast<Audio_branch*>(branch.get());
        b->signal_audio_pad_added().connect([this](std::shared_ptr<Pad> p){ _audio_pad_added.emit(p); });
    }

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
