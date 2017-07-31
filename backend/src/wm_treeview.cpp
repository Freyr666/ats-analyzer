#include "wm_treeview.hpp"
#include "errexpn.hpp"

using namespace Ats;
using namespace std;

void
Wm_treeview::add_window(shared_ptr<Ats::Wm_window> w) {
    uint stream = w->stream();
    uint pid = w->pid();

    _containers.try_emplace(make_tuple(stream,pid), unique_ptr<Wm_container>(new Wm_container(w)));
}

void
Wm_treeview::add_widget (std::tuple<uint,uint> pos, shared_ptr<Wm_widget> wdg) {
    auto nh = _containers.find(pos);
    if (nh != _containers.end()) nh->second->add_widget(wdg);
    else throw Error_expn("Wm_treeview: add_widget - no such window");
}

void
Wm_treeview::remove_window (std::tuple<uint,uint> pos) {
    _containers.erase(pos);
}

void
Wm_treeview::remove_widget (std::tuple<uint,uint> pos, std::tuple<uint,uint> wdg_pos) {
    auto nh = _containers.find(pos);
    if (nh != _containers.end()) nh->second->remove_widget(wdg_pos);
    else throw Error_expn("Wm_treeview: add_widget - no such window");
}
