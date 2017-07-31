#ifndef POSITION_H
#define POSITION_H

#include <string>
#include "json.hpp"

namespace Ats {

    using json = nlohmann::json;

    struct Position {
        uint x = 0;
        uint y = 0;
        uint width = 0;
        uint height = 0;

        bool operator== (const Position&);
        bool operator!= (const Position&);
        bool is_overlap (const Position&);
	std::string to_string () const;
    };
    
    void to_json(json& j, const Position&);
    void from_json(const json& j, Position&);
}

#endif /* POSITION_H */
