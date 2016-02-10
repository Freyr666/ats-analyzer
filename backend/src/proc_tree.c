#include "proc_tree.h"

PROC_TREE*
proc_tree_new(guint stream_id)
{
  PROC_TREE* rval;
  GstElement *queue, *parse, *fakesink;
  GstPadTemplate *tee_src_pad_template;
  GstPad *teepad, *sinkpad;
  
  /* init-ing tree */
  rval = g_new(PROC_TREE, 1);
  
  /* creating elements */
  rval->pipeline = gst_pipeline_new("proc-tree-pipe");
  rval->source = gst_element_factory_make("udpsrc", "proc-tree-source");
  queue = gst_element_factory_make("queue2", "proc-tree-queue");
  parse = gst_element_factory_make("tsparse", "proc-tree-parse");
  rval->faketee.tee = gst_element_factory_make("tee", "proc-tree-tee");
  rval->faketee.pad = NULL;
  fakesink = gst_element_factory_make("fakesink", NULL);
  rval->branches = NULL;
  
  /* init-ing tree metadata */
  rval->metadata = g_new(PROC_METADATA, 1);
  rval->metadata->stream_id = stream_id;
  rval->metadata->prog_info = NULL;
  rval->metadata->done = FALSE;
  /* setting queue length and buf size*/
  g_object_set (G_OBJECT (queue),
		"max-size-buffers", 2000000,
		"max-size-bytes", 429496729,
		NULL);
  
  /* linking pipeline */
  gst_bin_add_many(GST_BIN(rval->pipeline), rval->source, queue, parse, rval->faketee.tee, fakesink, NULL);
  gst_element_link_many (rval->source, queue, parse, rval->faketee.tee, NULL);

  /* connecting tee src to fakesink */
  sinkpad = gst_element_get_static_pad(fakesink, "sink");
  tee_src_pad_template = gst_element_class_get_pad_template(GST_ELEMENT_GET_CLASS (rval->faketee.tee), "src_%u");
  teepad = gst_element_request_pad(rval->faketee.tee, tee_src_pad_template, NULL, NULL);
  gst_pad_link(teepad, sinkpad);
  gst_object_unref(tee_src_pad_template);
  gst_object_unref(teepad);

  /* creating additional tee src pad for other brunches */
  tee_src_pad_template = gst_element_class_get_pad_template(GST_ELEMENT_GET_CLASS (rval->faketee.tee), "src_%u");
  rval->faketee.pad = gst_element_request_pad(rval->faketee.tee, tee_src_pad_template, NULL, NULL);
  gst_object_unref(tee_src_pad_template);
  
  return rval;
}

void
proc_tree_delete(PROC_TREE* this)
{
  proc_tree_remove_branches(this);
  gst_object_unref (GST_OBJECT (this->pipeline));
}

GstBus*
proc_tree_get_bus(PROC_TREE* this)
{
  return gst_pipeline_get_bus(GST_PIPELINE(this->pipeline));
}

void
proc_tree_set_source(PROC_TREE* this,
		     const gchar* srcpath,
		     const gchar* srcaddress,
		     const guint srcport )
{
  g_object_set (G_OBJECT (this->source),
		"port",        srcport,
		"address",     srcaddress,
		"uri",         srcpath,
		"buffer-size", 429496295,
		NULL);  
}

void
proc_tree_set_state(PROC_TREE* this,
		    GstState state)
{
  gst_element_set_state (this->pipeline, state);
}

void proc_tree_add_branches(PROC_TREE* this)
{
  guint channels;
  PROC_BRANCH* newbranch;
  GstPadTemplate *tee_src_pad_template;
  GstPad *teepad, *branchpad, *sinkpad;
  
  gst_element_set_state(this->pipeline, GST_STATE_PAUSED);
  /* if TS have been parsed */
  if (this->metadata->prog_info != NULL){
    channels = g_slist_length(this->metadata->prog_info);
    if(this->faketee.pad == NULL){
      g_printerr("proc_tree_add_branches: tee has no pad to connect!\n");
      return;
    }
    /* conecting channel tee pad to the graph */
    this->tee = gst_element_factory_make("tee", NULL);
    gst_bin_add(GST_BIN(this->pipeline), this->tee);
    sinkpad = gst_element_get_static_pad(this->tee, "sink");
    gst_pad_link(this->faketee.pad, sinkpad);

    /* adding processing branch for each channel */
    for (guint i = 0; i < channels; i++) {
	PROC_CH_DATA* tmpinfo = g_slist_nth_data(this->metadata->prog_info, i);
	if (!(tmpinfo->to_be_analyzed))
	  continue;
	newbranch = proc_branch_new(this->metadata->stream_id,
				    tmpinfo->number,
				    tmpinfo->xid);

	this->branches = g_slist_append(this->branches, newbranch);

	gst_bin_add(GST_BIN(this->pipeline), newbranch->bin);
	gst_bin_sync_children_states(GST_BIN(this->pipeline));
	/* connecting processing branch to channel tee */
	branchpad = gst_element_get_static_pad(newbranch->bin, "sink");
	tee_src_pad_template = gst_element_class_get_pad_template (GST_ELEMENT_GET_CLASS (this->tee), "src_%u");
	teepad = gst_element_request_pad (this->tee, tee_src_pad_template, NULL, NULL);
	gst_pad_link(teepad, branchpad);
	gst_object_unref (teepad);
	gst_object_unref (branchpad);
      }
  }
  gst_element_set_state(this->pipeline, GST_STATE_PLAYING);
}

void
proc_tree_remove_branches(PROC_TREE* this)
{
  guint blen;

  gst_element_set_state(this->pipeline, GST_STATE_NULL);
  /* deleting processing branch for each channel */
  blen = g_slist_length(this->branches);
  for (guint i = 0; i < blen; i++) {
    PROC_BRANCH* tmp = g_slist_nth_data(this->branches, i);
    if (tmp->bin != NULL)
      gst_bin_remove(GST_BIN(this->pipeline), tmp->bin);
    tmp->bin = NULL;
    proc_branch_delete(tmp);
  }
  /* deleting branches object and channel tee */
  g_slist_free(this->branches);
  this->branches = NULL;
  gst_bin_remove(GST_BIN(this->pipeline), this->tee);
  //gst_object_unref(this->tee);
  this->tee = NULL;
  gst_element_set_state(this->pipeline, GST_STATE_READY);
}
