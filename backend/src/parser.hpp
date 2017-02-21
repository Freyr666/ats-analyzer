#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <vector>
#include <experimental/optional>
#include <gstreamermm.h>
#include <glibmm.h>

#include <gst/gst.h>
#include <gst/mpegts/mpegts.h>
#include <glib.h>

namespace Ats {
    
    struct SIT {
	uint pmt_pid;
	long splice_time;
	uint ad;
    };

    namespace Parse {
/*
 * If section is of type sdt, pmt or pat, then true
 * else false
 */ 
	bool table (GstMpegtsSection * section, void* data);

/*
 * If section is of type sdt, then true
 */
	bool sdt   (GstMpegtsSection * section, void* data);

	bool scte  (GstMpegtsSection * section, SIT* data);

    };

};

#endif /* PARSER_H */
