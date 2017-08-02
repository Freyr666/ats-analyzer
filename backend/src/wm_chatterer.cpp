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
    std::function<void(const std::string&,const Wm_container&)> f = [&j_treeview](const std::string& s,const Wm_container& c) {
        json j_container = {s,shared_ptr<const Wm_container>(&c)};
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
Ats::to_json(json& j, const shared_ptr<const Wm_window> w) {
    /* Window type-independent fields */
    j = {{"enabled",w->is_enabled()},
         {"position",w->get_position()}};

    /* Window type-dependent fields */
    Wm_window::Type t = w->type();
    if (t == Wm_window::Type::Video) {
        const Wm_window_video* wv = dynamic_cast<const Wm_window_video*> (w.get());
        j["type"]    = "video";
        j["stream"]  = wv->stream();
        j["channel"] = wv->channel();
        j["pid"]     = wv->pid();
    }
    else {
        j["type"]    = "background";
    }
}

void
Ats::to_json(json& j, const shared_ptr<const Wm_widget> w) {
    /* Widget type-independent fields */
    j = {{"enabled",w->is_enabled()},
         {"position",w->get_position()}};

    /* Widget type-dependent fields */
    /* TODO */
}

void
Ats::to_json(json& j, const shared_ptr<const Wm_container> c) {
    std::function<void(const Wm_window&)> f_window = [&j](const Wm_window& w) {
        j = shared_ptr<const Wm_window>(&w);
    };
    std::function<void(const std::string&,const Wm_widget&)> f_widgets =
        [&j](const std::string& s, const Wm_widget& w) {
        json j_widget = {s,shared_ptr<const Wm_widget>(&w)};
        j["widgets"].push_back(j_widget);
    };
    c->apply(f_window);
    j["widgets"] = json::array();
    c->for_each(f_widgets);
}

void
Ats::to_json(json& j, const Wm_position& p) {
    j = {{"x",p.get_x()},
         {"y",p.get_y()},
         {"width",p.get_width()},
         {"height",p.get_height()}};
}
