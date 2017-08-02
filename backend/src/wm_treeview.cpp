#include "wm_treeview.hpp"
#include "errexpn.hpp"

#include <iostream>

using namespace Ats;
using namespace std;

// Wm_treeview::Wm_treeview (const std::map<std::string,std::shared_ptr<Wm_window>>& _windows,
//                           const std::map<std::string,std::shared_ptr<Wm_widget>>& _widgets,
//                           json& j) {

//     for (json::const_iterator it = j.cbegin(); it != j.cend(); ++it) {
//         const json j_window    = it.value().at(1);
//         const json j_widgets   = j_window.at("widgets");

//         std::string uid  = it.value().at(0).get<std::string>();
//         std::string type = j_window.at("type").get<std::string>();

//         auto wnd_it = _windows.find(uid);
//         if (wnd_it == windows.end())
//             throw Error_expn(std::string("Wm_treeview: window with uid '") + uid + "' not found");
//         std::shared_ptr<Wm_window> wnd = wnd_it->second;
//         if (wnd->type_to_string() != type)
//             throw Error_expn(std::string("Wm_treeview: bad window type - got '") + type + "', expected '"
//                              + wnd->type_to_string() + "'");

//         add_window(wnd);

//         for (json::const_iterator wdg_it = j_widgets.begin(); wdg_it != j_widgets.end(); ++ wdg_it) {
//             const json j_widget  = it.value().at(1);

//             std::string wdg_uid  = it.value().at(0).get<std::string>();
//             std::string wdg_type = j_widget.at("type").get<std::string>();

//             auto wdg_it = _widgets.find(wdg_uid);
//             if (wdg_it == _widgets.end())
//                 throw Error_expn(std::string("Wm_treeview: widget with uid '") + wdg_uid + "' not found")
//             std::shared_ptr<Wm_widget> wdg = wdg_it->second;
//             // TODO : check widget type

//             add_widget(uid, wdg);
//         }
//     }
// }

void
Wm_treeview::add_window(std::string uid, shared_ptr<Ats::Wm_window> w) {
    auto res = _containers.try_emplace(uid, unique_ptr<Wm_container>(new Wm_container(w)));
    if (!res.second) throw Error_expn("Wm_treeview: add_window - window already added");
}

void
Wm_treeview::add_widget (std::string wnd_uid, std::string wdg_uid, shared_ptr<Wm_widget> wdg) {
    auto nh = _containers.find(wnd_uid);
    if (nh == _containers.end()) {
        throw Error_expn("Wm_treeview: add_widget - no such window");
    }
    /* Check for this widget in other windows */
    std::function<void(const std::string&, const Wm_container&)> f =
        [&wnd_uid,&wdg_uid](const std::string& s, const Wm_container& c) {
        std::function<void(const std::string&, const Wm_widget&)> f_widg =
        [&wnd_uid,&wdg_uid](const std::string& s, const Wm_widget& w) {
            if (s == wdg_uid) {
                throw Error_expn(std::string("Wm_treeview: add_widget - widget with uid '") + wdg_uid +
                                 "' was already added to window with uid '" + wnd_uid + "'");
            }
        };
        c.for_each(f_widg);
    };
    for_each(f);

    nh->second->add_widget(wdg_uid,wdg);
}

void
Wm_treeview::remove_window (std::string pos) {
    _containers.erase(pos);
}

void
Wm_treeview::remove_widget (std::string pos, std::string wdg_pos) {
    auto nh = _containers.find(pos);
    if (nh != _containers.end()) nh->second->remove_widget(wdg_pos);
    else throw Error_expn("Wm_treeview: add_widget - no such window");
}

void
Wm_treeview::for_each (std::function<void(const std::string&, Wm_container&)>& f) {
    for (auto& nh : _containers) {
        f (nh.first , *nh.second);
    }
}

void
Wm_treeview::for_each (std::function<void(const std::string&, const Wm_container&)>& f) const {
    for (auto& nh : _containers) {
        f (nh.first , *nh.second);
    }
}

// Wm_container*
// Wm_treeview::find_container (std::string uid) {
    
// }

const Wm_container*
Wm_treeview::find_container (std::string uid) const {
    auto it = _containers.find(uid);
    if ( it != _containers.end()) {
        it->second;
    }
    return nullptr;
}
