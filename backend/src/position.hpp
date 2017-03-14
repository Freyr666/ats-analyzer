#ifndef POSITION_H
#define POSITION_H

#include <vector>

using namespace std;

namespace Ats {

    class Position {
    public:
	struct Entry {
	    int x_pos;
	    int y_pos;
	    int x_size;
	    int y_size;

	    Entry(int x,int y,int xs, int ys):x_pos(x),y_pos(y),x_size(xs),y_size(ys) {}
	};
	
    private:
	int row_counter = 0;
	int last_sqrt = 0;
        vector<Entry> positions;

    public:	    
	Position() {}

	pair<int,int> get_position(int size_x, int size_y);
	void reset() { positions.clear(); }
    };
    
};

#endif /* POSITION_H */
