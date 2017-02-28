#include "graph.hpp"

#include <iostream>

using namespace std;
using namespace Ats;

void
Graph::apply(const Options& o) {
    cout << o.to_string() << endl;;
}

void
Graph::set_state(Gst::State s) {
    pipe->set_state(s);
}

string
Graph::to_string() const {
    return "todo";
}
