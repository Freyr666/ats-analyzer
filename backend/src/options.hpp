#ifndef OPTIONS_H
#define OPTIONS_H

#include <vector>
#include <gstreamermm.h>

#include "metadata.hpp"
#include "probe.hpp"

using namespace std;

namespace Ats {

    struct Options {
	vector<Metadata> data;
	int              dummy;
    
	Options() {}
	~Options() {}

	void   connect(Probe& p) { p.updated.connect(
		sigc::mem_fun(*this, &Options::set_data));
	}

	void   set_data(const Metadata&);
	string to_string();
	string to_json();
	void   of_json(const string&);

	void operator=(const Metadata& m) { set_data(m); }
	void operator=(const string& js) { of_json(js); }
    };

};

#endif /* OPTIONS_H */
