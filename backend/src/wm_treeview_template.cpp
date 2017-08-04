#include "wm_treeview.hpp"

using namespace Ats;
using namespace std;

unique_ptr<Wm_treeview_template>
Wm_treeview_template::create (const json& j,
                              const std::map<std::string,shared_ptr<Wm_window>> _windows,
                              const std::map<std::string,shared_ptr<Wm_widget>> _widgets) {

    unique_ptr<Wm_treeview_template> tw(new Wm_treeview_template());

    for (json::const_iterator j_it = j.cbegin(); j_it != j.cend(); ++j_it) {
        const json j_window    = j_it.value().at(1);

        std::string uid  = j_it.value().at(0).get<std::string>();
        std::string type = j_window.at("type").get<std::string>();

        auto wnd_it = _windows.find(uid);
        if (wnd_it == _windows.end())
            throw Error_expn(elt_not_present("Window",uid));
        if (wnd_it->second->get_type_string() != type)
            throw Error_expn(elt_wrong_type("Window",uid,type,wnd_it->second->get_type_string()));

        Wm_position pos = parse_position(j_window.at("position"));
        tw->add_window(uid,
                       wnd_it->second->type(),
                       pos,
                       wnd_it->second);

        if (j.find("widgets") == j.end()) continue;
        const json j_widgets   = j_window.at("widgets");
        for (json::const_iterator j_wdg_it = j_widgets.begin(); j_wdg_it != j_widgets.end(); ++ j_wdg_it) {
            const json j_widget   = j_wdg_it.value().at(1);

            std::string wdg_uid   = j_wdg_it.value().at(0).get<std::string>();
            std::string wdg_type  = j_widget.at("type").get<std::string>();

            auto wdg_it = _widgets.find(wdg_uid);
            if (wdg_it != _widgets.end())
                throw Error_expn(elt_not_present("Widget",wdg_uid));
            if (wdg_it->second->get_type_string() != wdg_type)
                throw Error_expn(elt_wrong_type("Widget",wdg_uid,wdg_type,wdg_it->second->get_type_string()));

            Wm_position wdg_pos = parse_position(j_widget.at("position"));
            tw->add_widget(uid,
                           wdg_uid,
                           wdg_it->second->type(),
                           wdg_pos,
                           wdg_it->second);
        }
    }

    return tw;
}

void
Wm_treeview_template::validate (pair<uint,uint> res) const {
    for (auto it = _containers.begin(); it != _containers.end(); it++) {
        Wm_position pos = it->get_window().position;
        if ((uint)pos.get_rlc().first > res.first ||
            (uint)pos.get_rlc().second > res.second) {
            throw Error_expn(string("Window layout: part of the window '") + it->get_window().uid +
                             "' is located beyond screen borders");
        }
        // Windows' intersections
        if (it == (_containers.end() - 1)) continue;
        if (any_of(it++, _containers.end(), [&pos](auto& cont_it) {
                    Wm_position opos = cont_it.get_window().position;
                    return pos.is_overlap(opos);
                }) ) {
            throw Error_expn(string("Window layout: window ") + it->get_window().uid +
                             " is overlapping with another window");
        }
        // Widgets' intesections
        it->validate();
    }
}

void
Wm_treeview_template::add_window (string uid, Wm_window::Type type,
                                  Wm_position& pos, shared_ptr<Wm_window> window) {
    auto cont_it = find_if(_containers.begin(), _containers.end(), [&uid](Wm_container_template c) {
            return c.get_window().uid == uid;
        });
    if (cont_it != _containers.end()) throw Error_expn(elt_already_added("Window",uid));
    else {
        _containers.push_back(Wm_container_template(uid,type,pos,window));
    }
}

void
Wm_treeview_template::add_widget (string wnd_uid, string wdg_uid, Wm_widget::Type type,
                                  Wm_position& pos, shared_ptr<Wm_widget> widget) {
    auto cont_it = find_if(_containers.begin(), _containers.end(), [&wnd_uid](Wm_container_template c) {
            return c.get_window().uid == wnd_uid;
        });
    if (cont_it != _containers.end()) {
        /* Search if this widget was already added to some window */
        for_each(_containers.begin(), _containers.end(), [&wdg_uid](Wm_container_template c) {
                auto wdg_it = find_if(c.get_widgets().begin(), c.get_widgets().end(),
                                      [&wdg_uid](Wm_widget_template w) {
                                          return w.uid == wdg_uid;
                                      });
                if (wdg_it != c.get_widgets().end())
                    throw Error_expn(elt_already_added("Widget",wdg_uid,c.get_window().uid));
            });
        cont_it->add_widget(wdg_uid,type,pos,widget);
    }
    else
        throw Error_expn(string("Wm_treeview_template: add_widget - ") + elt_not_present("Window",wnd_uid));
}

Wm_position
Wm_treeview_template::parse_position (const json& j) {
    int left   = j.at("left");
    int top    = j.at("top");
    int right  = j.at("right");
    int bottom = j.at("bottom");
    std::pair<int,int> luc(left,top);
    std::pair<int,int> rlc(right,bottom);
    Wm_position pos(luc,rlc);
    return pos;
}

string
Wm_treeview_template::elt_not_present (string elt, string uid) {
    return elt + " '" + uid + "' not found";
}

string
Wm_treeview_template::elt_wrong_type (string elt, string uid, string type) {
    return elt + " '" + uid + "' has unknown type '" + type + "'";
}

string
Wm_treeview_template::elt_wrong_type (string elt, string uid, string type, string expected_type) {
    return elt + " '" + uid + "' has wrong type - got '" + type + "', expected '" + expected_type + "'";
}

string
Wm_treeview_template::elt_already_added (string elt, string uid, string wnd_uid) {
    string s = elt + " '" + uid + "' cannot be added more than once";
    if (!wnd_uid.empty()) s += " (already added to window '" + wnd_uid + "'";
    return s;
}

/* Wm_container_template */

void
Wm_treeview_template::Wm_container_template::add_widget(string uid, Wm_widget::Type type,
                                                        Wm_position& pos, shared_ptr<Wm_widget> widget) {
    auto wdg_it = find_if(_widgets.begin(), _widgets.end(), [&uid](Wm_widget_template w) {
            return w.uid == uid;
        });
    if (wdg_it != _widgets.end()) throw Error_expn(elt_already_added("Widget",uid,_window.uid));
    _widgets.push_back(Wm_widget_template{uid,type,pos,widget});
}

void
Wm_treeview_template::Wm_container_template::validate () const {
    Wm_position win_pos = _window.position;

    for (auto it = _widgets.begin(); it != _widgets.end(); it++) {
        Wm_position pos = it->position;
        if (pos.get_luc().first  < win_pos.get_luc().first  ||
            pos.get_luc().second < win_pos.get_luc().second ||
            pos.get_rlc().first  > win_pos.get_rlc().first  ||
            pos.get_rlc().second > win_pos.get_rlc().second ) {
            throw Error_expn(string("Widget layout: part of the widget '") + it->uid +
                             "' is located beyond win borders");
        }
        // Widgets' intersections
        if (it == (_widgets.end() - 1)) continue;
        if (any_of(it++, _widgets.end(), [&pos](auto& cont_it) {
                    Wm_position opos = cont_it.position;
                    return pos.is_overlap(opos);
                }) ) {
            throw Error_expn(string("Widget layout: widget '") + it->uid + "' is overlapping with another widget");
        }
    }
}

