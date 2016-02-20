#include <gst/gst.h>
#include <glib.h>

#include "ats_graph.h"

int
main(int argc,
     char *argv[])
{
  ATS_GRAPH* graph;
  gst_init(&argc, &argv);

  graph = ats_graph_new(0, "127.0.0.1", 1234);
  ats_graph_run(graph);
  ats_graph_delete(graph);
  
  return 0;
}

