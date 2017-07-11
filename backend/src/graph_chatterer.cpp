#include "graph.hpp"

using namespace std;
using namespace Ats;

static inline Gst::State
to_state(string& s) {
    if (s == "null") return Gst::STATE_NULL;
    if (s == "pause") return Gst::STATE_PAUSED;
    if (s == "play") return Gst::STATE_PLAYING;
    if (s == "stop") return Gst::STATE_NULL;
    // FIXME
    return Gst::STATE_NULL;
}

static inline string
from_state(Gst::State s) {
    if (s == Gst::STATE_NULL) return "stop";
    if (s == Gst::STATE_PAUSED) return "pause";
    if (s == Gst::STATE_PLAYING) return "play";
    return "null";
}

string
Graph::to_string() const {
    std::string rval = "State:\n\t\t";
    auto st = get_state();
    rval += from_state(st);
    rval += "\n\n";
    return rval;
}

json
Graph::serialize() const {
    auto st = get_state();
    json j = json{{"state", from_state(st)}};
    return j;
}

void
Graph::deserialize (const json& j) {
    constexpr const char* state_key = "state";

    /* if state key present in json */
    if (j.find(state_key) != j.end()) {
        auto sst = j.at(state_key).get<std::string>();
        auto st = to_state(sst);
        set_state(st);
    }
}
