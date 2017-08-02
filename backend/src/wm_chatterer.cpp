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

    json j_treeview = json::array();
    std::function<void(const std::string&,Wm_container&)> f = [&j_treeview](const std::string& s,Wm_container& c) {
        json j_container = {s,shared_ptr<Wm_container>(&c)};
        j_treeview.push_back(j_container);
    };
    _treeview.for_each(f);

    json j = json{{"background",{}},
                  {"resolution",{_resolution.first,_resolution.second}},
                  {"windows",j_windows},
                  {"widgets",j_widgets},
                  {"layout",j_treeview}};
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
    std::function<void(Wm_window&)> f_window = [&j](Wm_window& w) { j = shared_ptr<Wm_window>(&w); };
    std::function<void(const std::string&,Wm_widget&)> f_widgets = [&j](const std::string& s, Wm_widget& w) {
        json j_widget = {s,shared_ptr<Wm_widget>(&w)};
        j["widgets"].push_back(j_widget);
    };
    c->apply(f_window);
    j["widgets"] = json::array();
    c->for_each(f_widgets);
}
