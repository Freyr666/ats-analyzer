#include "position.hpp"

using namespace std;
using namespace Ats;

bool
Position::operator== (const Position& a) {
    return ((a.x == x) && (a.y == y) &&
            (a.width == width) && (a.height == height));
}

bool
Position::operator!= (const Position& a) {
    return !(*this == a);
}

bool
Position::is_overlap (const Position& a) {
    return ((a.x < (width + x)) && ((a.width + a.x) > x) &&
            (a.y < (height + y)) && ((a.height + a.y) > y));
}

string
Position::to_string () const {
    string rval = "X: ";
    rval += std::to_string(x);
    rval += " Y: ";
    rval += std::to_string(y);
    rval += " Width: ";
    rval += std::to_string(width);
    rval += " Height: ";
    rval += std::to_string(height);
    return rval;
}

void
Ats::to_json (json& j, const Position& p) {
    j = json{{"x", p.x}, {"y", p.y},
             {"width", p.width}, {"height", p.height}};
}

void
Ats::from_json(const json& j, Position& p) {
    p.x = j.at("x").get<uint>();
    p.y = j.at("y").get<uint>();
    p.width = j.at("width").get<uint>();
    p.height = j.at("height").get<uint>();
}
