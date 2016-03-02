#include <glib.h>
#include <gst/gst.h>
#include <gst/mpegts/mpegts.h>

#include "parse_ts.h"
#include "ats_branch.h"
#include "ats_metadata.h"
#include "ats_tree.h"
#include "ats_control.h"

#define DATA_MARKER 0x8BA820F0
#define VIDEO_DATA_MARKER 0xEA96C3D8

#define SDT_TIMEOUT 3

typedef struct __ats_graph
{
  GMainLoop* loop;
  ATS_TREE* tree; 
  ATS_CONTROL* control;
  /* Parsing vars*/
  time_t time;
  gboolean metadata_were_sent;
} ATS_GRAPH;

ATS_GRAPH* ats_graph_new(guint stream_id, gchar* ip, guint port);

void ats_graph_run(ATS_GRAPH* this);

void ats_graph_delete(ATS_GRAPH* this);
