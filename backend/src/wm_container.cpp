#include "wm_container.hpp"

#include <algorithm>

using namespace Ats;
using namespace std;

Wm_container::~Wm_container() {
    _window->disable();
    std::for_each(_widgets.begin(), _widgets.end(), [](pair<const string,std::shared_ptr<Wm_widget>>& wp)
                  {  wp.second->disable(); });
}

void
Wm_container::add_widget(shared_ptr<Wm_widget> wdg) {
    _widgets.try_emplace(wdg->gen_name(),wdg);
    wdg->enable();
}

void
Wm_container::remove_widget (string pos) {
    auto nh = _widgets.extract(pos);
    if (nh) nh.mapped()->disable();
}

void
Wm_container::for_each (std::function<void(const std::string&,Wm_widget&)>& f) {
    for (auto& nh : _widgets) {
        f (nh.first, *nh.second);
    }
}

void
Wm_container::validate () {
    Wm_position win_pos = _window->get_position();
    
    for (auto it = _widgets.begin(); it != _widgets.end(); it++) {
        Wm_position pos = it->second->get_position();
        if (pos.get_luc().first  < win_pos.get_luc().first  ||
            pos.get_luc().second < win_pos.get_luc().second ||
            pos.get_rlc().first  > win_pos.get_rlc().first  ||
            pos.get_rlc().second > win_pos.get_rlc().second ) {
            throw Error_expn("Widget layout: part of the widget " + it->first + " is located beyond win borders");
        }
        // Widgets' intersections
        if (any_of(it++, _widgets.end(), [&pos](auto& cont_it) {
                    Wm_position opos = cont_it.second->get_position();
                    return pos.is_overlap(opos);
                }) ) {
            throw Error_expn("Widget layout: widget " + it->first + " is overlapping with another widget");
        }
    }
}
