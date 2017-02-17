#include "ats_branch.h"
#include "ats_subbranch.h"
#include <stdlib.h>

typedef struct __callback_data
{
  ATS_METADATA* data;
  ATS_BRANCH* branch;
  guint xid;
  double volume;
} CALLBACK_DATA;

static void
branch_added (GstBin     *bin,
	      GstElement *element,
	      gpointer    user_data)
{
  gst_element_set_state(GST_ELEMENT(bin), GST_STATE_PLAYING);
  gst_bin_sync_children_states(GST_BIN(bin));
}

static void
branch_on_pad_added(GstElement* el,
		    GstPad* pad,
		    gpointer data)
{
  CALLBACK_DATA*   cb_data;
  GstCaps*         caps;
  GstStructure*    pad_struct;
  GstPad*          sinkpad;
  const gchar*     pad_type;
  guint            pid_num;
  ATS_SUBBRANCH*   tail;
  gchar**          type_tocs;
  gchar**          pid_tocs;
  ATS_BRANCH*      branch;
  ATS_METADATA*    metadata;
  ATS_PID_DATA*    piddata;
  GError*          tmp_error;
  
  cb_data    =  (CALLBACK_DATA*)data;
  pad_struct =  NULL;
  pad_type   =  NULL;
  pid_num    =  0;
  tail       =  NULL;
  branch     =  cb_data->branch;
  metadata   =  cb_data->data;
  tmp_error  =  NULL;
  
  g_print ("Received new pad '%s' from '%s':\n",
	   GST_PAD_NAME (pad),
	   GST_ELEMENT_NAME (el));
  
  /* Getting new pad's caps */
  caps = gst_pad_get_current_caps(pad);
  pad_struct = gst_caps_get_structure (caps, 0);
  pad_type = gst_structure_get_name (pad_struct);
  g_print("Result: %s\n", pad_type);

  /* Getting pad's type and pid */
  pid_tocs = g_strsplit(GST_PAD_NAME (pad), "_", 3);
  pid_num = strtoul(pid_tocs[2], NULL, 16);
  type_tocs = g_strsplit(pad_type, "/", 2);
  g_print("Got %s of type %s\n", type_tocs[0], type_tocs[1]);
  
  /* Finding pad's pid in metadata */
  piddata = ats_metadata_find_pid(metadata,
				  branch->prog_num,
				  pid_num);

  if (piddata && piddata->to_be_analyzed){
    /* If recieved pad is video pad: */
    if (g_strcmp0(type_tocs[0], "video") == 0) {
      tail = create_video_subbranch(type_tocs[1],
				    branch->stream_id,
				    branch->prog_num,
				    pid_num,
				    cb_data->xid,
				    &tmp_error);
      if (tail == NULL) goto finally;
    }
    /* Else if pad is audio pad: */
    else if (g_strcmp0(type_tocs[0], "audio") == 0) {
      tail = create_audio_subbranch(type_tocs[1],
				    branch->stream_id,
				    branch->prog_num,
				    pid_num,
				    cb_data->volume,
				    &tmp_error);
      if (tail == NULL) goto finally;
    }
    
    /* Connecting subbranch to the tsdemux element of the branch: */
    g_print("Playing pipeline has been created\n");
    branch->subbranches = g_slist_append(branch->subbranches,
					 tail);

    /* Connecting pads: */
    g_print("Playing pipeline has been created\n");

    gst_element_set_state(tail->bin, GST_STATE_PLAYING);
    gst_bin_sync_children_states(GST_BIN(tail->bin));
    gst_bin_add((GstBin*) branch->bin, tail->bin);

    sinkpad = gst_element_get_static_pad (tail->bin, "sink");
    gst_pad_link (pad, sinkpad);
    g_print("Linked!\n");
            
    gst_object_unref(GST_OBJECT(sinkpad));
    /*
     * Graph printing:
     * GstElement* tmp = gst_element_get_parent(branch->bin);
     * GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN(tmp), 
     *                           GST_DEBUG_GRAPH_SHOW_ALL, 
     *                           "pipeline");
     */
  }
  
 finally:
  g_strfreev(pid_tocs);
  g_strfreev(type_tocs);
  if (tmp_error) {
    g_printerr(tmp_error->message);
  }
}

ATS_BRANCH*
ats_branch_new(const guint stream_id,
	       const guint prog_num,
	       const guint xid,
	       const double volume,
	       ATS_METADATA* data,
	       GError** error)
{
  ATS_BRANCH     *rval;
  GstPad         *pad, *ghost_pad;
  GstElement     *queue, *demux;
  CALLBACK_DATA  *cb_data;
  
  rval = g_new(ATS_BRANCH, 1);
  rval->stream_id = stream_id;
  rval->prog_num = prog_num;
  rval->bin = gst_bin_new(NULL);
  rval->subbranches = NULL;

  cb_data = g_new(CALLBACK_DATA, 1);
  cb_data->branch = rval;
  cb_data->data = data;
  cb_data->xid = xid;
  cb_data->volume = volume;
  
  queue = gst_element_factory_make("queue2", NULL);
  demux = gst_element_factory_make("tsdemux", NULL);
  if (queue == NULL || demux == NULL) {
    goto error;
  }

  g_object_set(G_OBJECT(demux),
	       "emit-stats", TRUE,
	       "program-number", rval->prog_num,
	       NULL);
  
  g_object_set (G_OBJECT (queue),
		"max-size-buffers", 200000,
		"max-size-bytes", 429496729,
		NULL);
  
  gst_bin_add_many(GST_BIN(rval->bin), queue, demux, NULL);
  gst_element_link_many(queue, demux, NULL);
  pad = gst_element_get_static_pad (queue, "sink");
  ghost_pad = gst_ghost_pad_new ("sink", pad);
  gst_pad_set_active (ghost_pad, TRUE);
  gst_element_add_pad (rval->bin, ghost_pad);
  gst_object_unref (pad);

  g_signal_connect(demux, "pad-added",
		   G_CALLBACK (branch_on_pad_added), cb_data);
  g_signal_connect(rval->bin, "element-added",
		   G_CALLBACK (branch_added), NULL);
  
  return rval;

 error:
  if (queue)     gst_object_unref(queue);
  if (demux)     gst_object_unref(demux);
  if (cb_data)   g_free(cb_data);
  if (rval->bin) gst_object_unref(rval->bin);
  if (rval)      g_free(rval);
  g_set_error(error,
	      G_ERR_UNKNOWN, -1,
	      "Error: failed to create branch");
  return NULL;
}

void
ats_branch_delete(ATS_BRANCH* this)
{
  if (this->bin){
    gst_object_unparent(GST_OBJECT(this->bin));
    gst_object_unref(GST_OBJECT(this->bin));
  }
  g_free(this);
}

