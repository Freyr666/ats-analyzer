#include "wm_treeview.hpp"
#include "errexpn.hpp"
#include <algorithm>

using namespace Ats;
using namespace std;

void
Wm_treeview::reset() {
    _containers.clear();
}

void
Wm_treeview::reset_from_template(Wm_treeview_template* t) {
    reset();
    for (auto& wnd_it : t->get_containers()) {
        add_window(wnd_it.get_window().uid, wnd_it.get_window().window);
        wnd_it.get_window().window->set_position(wnd_it.get_window().position);
        for (auto& wdg_it : wnd_it.get_widgets()) {
            add_widget(wnd_it.get_window().uid, wdg_it.uid, wdg_it.widget);
            wdg_it.widget->set_position(wdg_it.position);
        }
    }
}

void
Wm_treeview::add_window(std::string uid,shared_ptr<Ats::Wm_window> w) {
    _containers.try_emplace(uid, unique_ptr<Wm_container>(new Wm_container(w)));
}

void
Wm_treeview::add_widget (std::string wnd_uid, std::string wdg_uid, shared_ptr<Wm_widget> wdg) {
    auto nh = _containers.find(wnd_uid);
    if (nh != _containers.end()) nh->second->add_widget(wdg_uid,wdg);
    else throw Error_expn("Wm_treeview: add_widget - no such window");
}

void
Wm_treeview::remove_window (std::string pos) {
    _containers.erase(pos);
}

void
Wm_treeview::remove_widget (std::string pos, std::string wdg_pos) {
    auto nh = _containers.find(pos);
    if (nh != _containers.end()) nh->second->remove_widget(wdg_pos);
    else throw Error_expn("Wm_treeview: remove_widget - no such window");
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
