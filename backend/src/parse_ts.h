#ifndef PARSE_TS_H
#define PARSE_TS_H

#include <gst/gst.h>
#include <gst/mpegts/mpegts.h>
#include <glib.h>

void parse_table (GstMpegtsSection * section, void* data);

void parse_sdt (GstMpegtsSection * section, void* data);

#endif /* PARSE_TS_H */
