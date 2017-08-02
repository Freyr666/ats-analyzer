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
Wm_treeview::add_window(shared_ptr<Ats::Wm_window> w) {
    _containers.try_emplace(w->gen_name(), unique_ptr<Wm_container>(new Wm_container(w)));
}

void
Wm_treeview::add_widget (std::string pos, shared_ptr<Wm_widget> wdg) {
    auto nh = _containers.find(pos);
    if (nh != _containers.end()) nh->second->add_widget(wdg);
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
    else throw Error_expn("Wm_treeview: add_widget - no such window");
}

void
Wm_treeview::for_each (std::function<void(const std::string&, Wm_container&)>& f) {
    for (auto& nh : _containers) {
        f (nh.first , *nh.second);
    }
}

void
Wm_treeview::validate (pair<uint,uint> res) {
    for (auto it = _containers.begin(); it != _containers.end(); it++) {
        Wm_position pos = it->second->get_window()->get_position();
        if ((uint)pos.get_rlc().first > res.first ||
            (uint)pos.get_rlc().second > res.second) {
            throw Error_expn("Window layout: part of the window " + it->first + " is located beyond screen borders");
        }
        // Windows' intersections
        if (any_of(it++, _containers.end(), [&pos](auto& cont_it) {
                    Wm_position opos = cont_it.second->get_window()->get_position();
                    return pos.is_overlap(opos);
                }) ) {
            throw Error_expn("Window layout: window " + it->first + " is overlapping with another window");
        }
        // Widgets' intesections
        it->second->validate();
    }
}
