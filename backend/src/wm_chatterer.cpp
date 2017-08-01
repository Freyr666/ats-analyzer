#include "wm.hpp"

using namespace std;
using namespace Ats;

string
Wm::to_string() const {
    return "TODO";
}

json
Wm::serialize() const {
    std::vector<pair<std::string,shared_ptr<Wm_window>>> windows_v(_windows.begin(), _windows.end());
    std::vector<pair<std::string,shared_ptr<Wm_widget>>> widgets_v(_widgets.begin(), _widgets.end());
    json j_windows(windows_v);
    json j_widgets(widgets_v);
    json j = json{{"background",{}},
                  {"resolution",{_resolution.first,_resolution.second}},
                  {"windows",j_windows},
                  {"widgets",j_widgets},
                  {"layout",_treeview.serialize()}};
    return j;
}

void
Wm::deserialize(const json&) {

}

void
Ats::to_json(json& j, const shared_ptr<Wm_window> w) {
    Wm_window::Type t = w->type();
    if (t == Wm_window::Type::Video) {
        Wm_window_video* wv = dynamic_cast<Wm_window_video*> (w.get());
        j = {{"stream",wv->stream()},
             {"channel",wv->channel()},
             {"pid",wv->pid()},
             {"enabled",wv->is_enabled()},
             {"position",{}}};
    }
    else {
        j = {};
    }
}

void
Ats::to_json(json& j, const shared_ptr<Wm_widget> w) {
    /* TODO */
    j = {};
}

void
Ats::to_json(json& j, const shared_ptr<Wm_container> c) {
    json j;
    j = c->_window;
    j["widgets"] = c->_widgets;
}
