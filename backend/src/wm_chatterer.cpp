#include "wm.hpp"

using namespace std;
using namespace Ats;

string
Wm::to_string() const {
    return "TODO";
}

json
Wm::serialize() const {
    typedef std::function<void(const std::string&,const Wm_container&)> f_containers_t;

    std::vector<pair<std::string,shared_ptr<const Wm_window>>> windows_v(_windows.begin(), _windows.end());
    std::vector<pair<std::string,shared_ptr<const Wm_widget>>> widgets_v(_widgets.begin(), _widgets.end());
    json j_windows(windows_v);
    json j_widgets(widgets_v);

    json j_treeview;
    f_containers_t f = [&j_treeview](const std::string& s,const Wm_container& c) {
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
Wm::deserialize(const json& j) {
    constexpr const char* wm_bg_key         = "background";
    constexpr const char* wm_resolution_key = "resolution";
    constexpr const char* wm_layout_key     = "layout";

    // validate();

    if (j.find(wm_bg_key) != j.end()) {
        const json j_background = j.at(wm_bg_key);
        /* TODO */
    }
    if (j.find(wm_resolution_key) != j.end()) {
        const json j_resolution = j.at(wm_resolution_key);
        uint width  = j_resolution.at(0).get<uint>();
        uint height = j_resolution.at(1).get<uint>();
        const resolution_t p(width,height);
        set_resolution(p);
    }
    if (j.find(wm_layout_key) != j.end()) {
        const json j_layout = j.at(wm_layout_key);
        unique_ptr<Wm_treeview_template> tw_template = Wm_treeview_template::create(j_layout,_windows,_widgets);
        tw_template->validate(_resolution);
        _treeview.reset_from_template(tw_template.get());
    }

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
    typedef std::function<void(const std::string&,const Wm_widget&)> f_widgets_t;

    json j_widgets;
    f_widgets_t f_widgets = [&j_widgets](const std::string& s, const Wm_widget& w) {
        json j_widget = {s,shared_ptr<const Wm_widget>(&w)};
        j_widgets.push_back(j_widget);
    };

    j = c->get_window();
    c->for_each(f_widgets);
    j["widgets"] = j_widgets;
}

void
Ats::to_json(json& j, const Wm_position& p) {
    j = {{"x",p.get_x()},
         {"y",p.get_y()},
         {"width",p.get_width()},
         {"height",p.get_height()}};
}
