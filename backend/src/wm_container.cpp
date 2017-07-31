#include "wm_container.hpp"

#include <algorithm>

using namespace Ats;
using namespace std;

Wm_container::~Wm_container() {
    _window->disable();
    for_each(_widgets.begin(), _widgets.end(), [](pair<const string,std::shared_ptr<Wm_widget>>& wp)
	     {  wp.second->disable();   });
}

void
Wm_container::add_widget(shared_ptr<Wm_widget> wdg) {
    _widgets.try_emplace(wdg->name(),wdg);
    wdg->enable();
}

void
Wm_container::remove_widget (string pos) {
    auto nh = _widgets.extract(pos);
    if (nh) nh.mapped()->disable();
}
