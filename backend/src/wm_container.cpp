#include "wm_container.hpp"

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
Wm_container::apply (std::function<void(Wm_window&)>& f) {
    f(*_window);
}

void
Wm_container::apply (std::function<void(const Wm_window&)>& f) const {
    f(*_window);
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
