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
        add_container(wnd_it.name, std::shared_ptr<Wm_container>(new Wm_container(wnd_it.name, wnd_it.position)));
        for (auto& wdg_it : wnd_it.get_widgets()) {
            add_widget(wnd_it.name, wdg_it.uid, wdg_it.widget);
            wdg_it.widget->set_position(wdg_it.position);
        }
    }
}

void
Wm_treeview::add_container(std::string uid,shared_ptr<Ats::Wm_container> c) {
    _containers.try_emplace(uid, c);
}

void
Wm_treeview::add_widget (std::string wnd_uid, std::string wdg_uid, shared_ptr<Wm_widget> wdg) {
    auto nh = _containers.find(wnd_uid);
    if (nh != _containers.end()) nh->second->add_widget(wdg_uid,wdg);
    else throw Error_expn("Wm_treeview: add_widget - no such window");
}

void
Wm_treeview::remove_container (std::string pos) {
    _containers.erase(pos);
}

void
Wm_treeview::remove_widget (std::string pos, std::string wdg_pos) {
    auto nh = _containers.find(pos);
    if (nh != _containers.end()) nh->second->remove_widget(wdg_pos);
    else throw Error_expn("Wm_treeview: remove_widget - no such window");
}

void
Wm_treeview::remove_widget (std::string wdg_pos) {
    for (auto& nh : _containers) {
        nh.second->remove_widget(wdg_pos);
        if (nh.second->empty()) _containers.erase(nh.first);
    }
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
