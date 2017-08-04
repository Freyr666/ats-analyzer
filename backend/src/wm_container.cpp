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
Wm_container::add_widget(std::string uid, shared_ptr<Wm_widget> wdg) {
    _widgets.try_emplace(uid,wdg);
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
Wm_container::for_each (std::function<void(const std::string&,const Wm_widget&)>& f) const {
    for (auto& nh : _widgets) {
        f (nh.first, *nh.second);
    }
}
