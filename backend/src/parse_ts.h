#ifndef PARSE_TS_H
#define PARSE_TS_H

#include <gst/gst.h>
#include <gst/mpegts/mpegts.h>
#include <glib.h>

/*
 * If section is of type sdt, pmt or pat, then true
 * else false
 */ 
gboolean parse_table (GstMpegtsSection * section, void* data);

/*
 * If section is of type sdt, then true
 */
gboolean parse_sdt (GstMpegtsSection * section, void* data);

gboolean parse_scte(GstMpegtsSection * section, void* data);

#endif /* PARSE_TS_H */
