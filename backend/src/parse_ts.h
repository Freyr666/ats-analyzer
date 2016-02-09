#ifndef PARSE_TS_H
#define PARSE_TS_H

#include <gst/gst.h>
#include <gst/mpegts/mpegts.h>
#include <glib.h>

gboolean parse_table (GstMpegtsSection * section, void* data);

#endif /* PARSE_TS_H */
