#ifndef WM_CONFIG_H
#define WM_CONFIG_H

#include <map>
#include <list>

using namespace std;
using namespace Glib;

namespace Ats {

    class Wm_config {
    public:
	struct Widget {
	    uint stream;
	    uint pid;
	    string name;
	    pair<int,int> luc;
	    pair<int,int> rlc;
	};
	struct Window {
	    uint stream;
	    uint pid;
	    string name;
	    pair<int,int> luc;
	    pair<int,int> rlc;
	    list<Widget>  childs;
	};

	
    private:
	pair<uint,uint> resolution = make_pair(1920, 1080);
	map<pair<uint,uint>,Widget> avail_widgets;
	map<pair<uint,uint>,Window> avail_windows;
	list<Window> active_windows;
    };
    
}

#endif /* WM_CONFIG_H */
