#include <gst/gst.h>
#include <glib.h>

#include "ats_graph.h"

int
main(int argc,
     char *argv[])
{
  GError* local_error;
  ATS_GRAPH* graph;
  guint port = 1234;
  guint stream = 0;
  gchar* const ip_default = "127.0.0.1";
  gchar* ip = ip_default;
  
  gst_init(&argc, &argv);

  local_error = NULL;
  
  for (int i = 1; i < argc; i++) {
    if (g_strcmp0(argv[i], "--port") == 0){
      port = g_strtod(argv[i+1], NULL);
      i++;
    }
    if (g_strcmp0(argv[i], "--ip") == 0){
      ip = argv[i+1];
      i++;
    }
    if (g_strcmp0(argv[i], "--stream") == 0){
      stream = g_strtod(argv[i+1], NULL);
      i++;
    }
    if (g_strcmp0(argv[i], "--help") == 0){
      g_print("Usage: %s [options args]\n", argv[0]);
      g_print("Options:\n");
      g_print("--stream stream_id\n");
      g_print("--ip address\n");
      g_print("--port port\n");
      g_print("--help\n");
      g_print("Default values:\n");
      g_print("stream_ip: 0\taddress: 127.0.0.1\tport: 1234\n");
      return 0;
    }
  }

  graph = ats_graph_new(stream, ip, port, &local_error);
  ats_graph_run(graph);
  ats_graph_delete(graph);
  
  return 0;
}

