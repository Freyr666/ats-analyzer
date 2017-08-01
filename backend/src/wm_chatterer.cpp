#include "wm.hpp"

using namespace std;
using namespace Ats;

string
Wm::to_string() const {
    return "TODO";
}

json
Wm::serialize() const {
    json j_windows(_windows);
    json j_widgets(_widgets);
    json j = json{{"background",{}},
                  {"resolution",{_resolution.first,_resolution.second}},
                  {"windows",j_windows,
                  {"widgets",j_widgets,
                  {"layout",_treeview};
    return j;
}

void
Wm::deserialize(const json&) {

}

void
Ats::to_json(json& j, const Wm_window& w) {
    j = {{"stream",w.stream()},
         {"channel",w.channel()},
         {"pid",w.pid()},
         {"enabled",w.is_enabled()},
         {"position",{}}};
}

void
Ats::to_json(json& j, const Wm_widget& w) {
    j = {};
}

void
Ats::to_json(json& j, const Wm_treeview& tw) {
    j = {};
}
