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
Wm::validate_layout(const json& j) {

    vector<std::string> window_uids;
    vector<pair<std::string,std::string>> widget_uids;

    for (json::const_iterator it = j.cbegin(); it != j.cend(); ++it) {
        const json j_window    = it.value().at(1);
        const json j_widgets   = j_window.at("widgets");

        std::string uid  = it.value().at(0).get<std::string>();
        std::string type = j_window.at("type").get<std::string>();

        std::shared_ptr<Wm_window> wnd = find_window(uid);
        if (wnd->type_to_string() != type)
            throw Error_expn(std::string("Bad window type - got '") + type + "', expected '"
                             + wnd->type_to_string() + "'");
        if (find(window_uids.begin(), window_uids.end(), uid) != window_uids.end()) {
            throw Error_expn(std::string("Window with uid '") + uid + "' cannot be added more than once");
        }

        for (json::const_iterator wdg_it = j_widgets.begin(); wdg_it != j_widgets.end(); ++ wdg_it) {
            const json j_widget  = it.value().at(1);

            std::string wdg_uid  = it.value().at(0).get<std::string>();
            std::string wdg_type = j_widget.at("type").get<std::string>();

            std::shared_ptr<Wm_widget> wdg = find_widget(wdg_uid);
            auto res = std::find_if(widget_uids.begin(), widget_uids.end(),
                                    [&wdg_uid](pair<std::string,std::string> p) -> bool {
                                        return p.second == wdg_uid;
                                    });
            if(res != widget_uids.end()) {
                throw Error_expn(std::string("Widget with uid '") + res->second +
                                 "' cannot be added more than once (already added to window with uid '" +
                                 res->first + "')");
            }

            pair<std::string,std::string> p(uid,wdg_uid);
            widget_uids.push_back(p);
        }

        window_uids.push_back(uid);
    }
}

void
Wm::deserialize(const json& j) {
    typedef std::function<void(const Wm_container&)> find_container_t;

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
        validate_layout(j_layout);
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
    typedef std::function<void(const Wm_window&)> f_window_t;
    typedef std::function<void(const std::string&,const Wm_widget&)> f_widgets_t;

    json j_widgets;
    f_window_t f_window = [&j](const Wm_window& w) { j = shared_ptr<const Wm_window>(&w); };
    f_widgets_t f_widgets = [&j_widgets](const std::string& s, const Wm_widget& w) {
        json j_widget = {s,shared_ptr<const Wm_widget>(&w)};
        j_widgets.push_back(j_widget);
    };

    c->apply(f_window);
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
