#include "branch.hpp"

using namespace Ats;
using namespace std;

Pad::Pad(Glib::RefPtr<Gst::Pad>& p) {
    auto pname = p->get_name();
    vector<Glib::ustring> name_toks = Glib::Regex::split_simple("_", pname);

    auto& type = name_toks[1];

    if ( type == "video" ) _t = Pad::Type::Video;
    else if ( type == "audio" ) _t = Pad::Type::Audio;
    else _t = Pad::Type::Unknown;

    _pad = p;
}
