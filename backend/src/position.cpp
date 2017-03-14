#include "position.hpp"

#include <algorithm>

using namespace std;
using namespace Ats;

static inline pair<bool,int>
is_sqrt(int x) {
    if (x == 0) return pair<bool,int>(true,0);
    for (int i = 1; i < (x/2); i++) {
	if (x == (i*i)) return pair<bool,int>(true,i);
    }
    return pair<bool,int>(false,0);
}

static inline int
get_x(Position::Entry e) {
    return e.x_pos + e.x_size;
}

static inline int
get_y(Position::Entry e) {
    return e.y_pos + e.y_size;
}

static inline int
max_v(vector<Position::Entry> v, int(*getter)(Position::Entry)) {
    int result = 0;
    for_each(v.begin(), v.end(), [&result,&getter](Position::Entry e){
	    int tmp = getter(e);
	    if (tmp > result) result = tmp;
	});
    return result;
}

pair<int,int>
Position::get_position(int size_x, int size_y) {
    auto size = positions.size();
    pair<bool,int> sqrt = is_sqrt(size);
    /*
     *    * *[*]
     *    * *
     */
    if (sqrt.first) {
	last_sqrt = sqrt.second;
	row_counter = sqrt.second - 1;
	int x = max_v(positions,get_x);
	int y = 0;
	positions.push_back(Entry(x, y, size_x, size_y));
	return pair<int,int>(x,y);
    }
    /*
     *   * * *
     *   * *[*]
     *   * *
     */
    if (row_counter >= 0) {
	row_counter -= 1;
	int pos_dif = row_counter == 0 ? last_sqrt : 2*last_sqrt - 1;
	auto e = positions[size-pos_dif];
	int x = e.x_pos + e.x_size;
	int y = e.y_pos;
	positions.push_back(Entry(x, y, size_x, size_y));
	return pair<int,int>(x,y);
    }
    /*
     *  * *
     * [*]
     */
    if (size == (uint)(last_sqrt*(last_sqrt+1))) {
	int x = 0;
	int y = max_v(positions,get_y);
	positions.push_back(Entry(x, y, size_x, size_y));
	return pair<int,int>(x,y);
    } else {
	auto e = positions.back();
	int x = e.x_pos + e.x_size;
	int y = max_v(positions,get_y);
	positions.push_back(Entry(x, y, size_x, size_y));
	return pair<int,int>(x,y);
    }
}
