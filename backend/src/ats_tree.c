#include "ats_tree.h"
#include "unistd.h"

ATS_TREE*
ats_tree_new(guint stream_id,
	     gchar* ip,
	     guint port)
{
  ATS_TREE* rval;
  GstElement *parse, *fakesink;
  GstPadTemplate *tee_src_pad_template;
  GstPad *teepad, *sinkpad;
  
  /* init-ing tree */
  rval = g_new(ATS_TREE, 1);
  
  /* creating elements */
  rval->pipeline = gst_pipeline_new("proc-tree-pipe");
  rval->source = gst_element_factory_make("udpsrc", "proc-tree-source");
  parse = gst_element_factory_make("tsparse", "proc-tree-parse");
  rval->faketee.tee = gst_element_factory_make("tee", "proc-tree-tee");
  rval->faketee.pad = NULL;
  fakesink = gst_element_factory_make("fakesink", NULL);
  rval->branches = NULL;
  
  /* init-ing tree metadata */
  rval->metadata = ats_metadata_new(stream_id);
  /* setting udpsrc port and buf size*/
  g_object_set (G_OBJECT (rval->source),
		"timeout", 5000000000,
		"port",        port,
		"address",     ip,
		NULL);  
  
  /* linking pipeline */
  gst_bin_add_many(GST_BIN(rval->pipeline),
		   rval->source,
		   parse,
		   rval->faketee.tee,
		   fakesink, NULL);
  gst_element_link_many (rval->source, parse, rval->faketee.tee, NULL);

  /* connecting tee src to fakesink */
  sinkpad = gst_element_get_static_pad(fakesink, "sink");
  tee_src_pad_template = gst_element_class_get_pad_template(GST_ELEMENT_GET_CLASS (rval->faketee.tee),
							    "src_%u");
  teepad = gst_element_request_pad(rval->faketee.tee,
				   tee_src_pad_template,
				   NULL, NULL);
  gst_pad_link(teepad, sinkpad);
  gst_object_unref(tee_src_pad_template);
  gst_object_unref(teepad);

  /* creating additional tee src pad for other brunches */
  tee_src_pad_template = gst_element_class_get_pad_template(GST_ELEMENT_GET_CLASS(rval->faketee.tee),
							    "src_%u");
  rval->faketee.pad = gst_element_request_pad(rval->faketee.tee,
					      tee_src_pad_template,
					      NULL, NULL);
  gst_object_unref(tee_src_pad_template);
  
  return rval;
}

void
ats_tree_delete(ATS_TREE* this)
{
  ats_tree_remove_branches(this);
  gst_object_unref (GST_OBJECT (this->pipeline));
}

GstBus*
ats_tree_get_bus(ATS_TREE* this)
{
  return gst_pipeline_get_bus(GST_PIPELINE(this->pipeline));
}

void
ats_tree_set_state(ATS_TREE* this,
		    GstState state)
{
  gst_element_set_state (this->pipeline, state);
}

ATS_SUBBRANCH*
ats_tree_find_subbranch(ATS_TREE* this,
			guint prog,
			guint pid)
{
  guint len;
  guint pids;
  if(this->branches == NULL) return NULL;
  len = g_slist_length(this->branches);
  for (guint i = 0; i < len; i++) {
    ATS_BRANCH* tmp_branch = g_slist_nth_data(this->branches, i);
    if (tmp_branch->prog_num != prog) continue;
    if (tmp_branch->subbranches == NULL) return NULL;
    pids = g_slist_length(tmp_branch->subbranches);
    for (guint j = 0; j < pids; j++) {
      ATS_SUBBRANCH* tmp_subbranch = g_slist_nth_data(tmp_branch->subbranches, j);
      if (tmp_subbranch->pid != pid) continue;
      return tmp_subbranch;
    }
    return NULL;
  }
  return NULL;
}

void ats_tree_add_branches(ATS_TREE* this)
{
  guint channels;
  ATS_BRANCH* newbranch;
  GstPadTemplate *tee_src_pad_template;
  GstPad *teepad, *branchpad, *sinkpad;
  g_printerr("ats_tree_add_branches!\n");
  gst_element_set_state(this->pipeline, GST_STATE_PAUSED);
  /* if TS have been parsed */
  if (this->metadata->prog_info != NULL){
    channels = g_slist_length(this->metadata->prog_info);
    if(this->faketee.pad == NULL){
      g_printerr("ats_tree_add_branches: tee has no pad to connect!\n");
      return;
    }
    /* conecting channel tee pad to the graph */
    this->tee = gst_element_factory_make("tee", NULL);
    gst_bin_add(GST_BIN(this->pipeline), this->tee);
    sinkpad = gst_element_get_static_pad(this->tee, "sink");
    gst_pad_link(this->faketee.pad, sinkpad);

    /* adding processing branch for each channel */
    for (guint i = 0; i < channels; i++) {
	ATS_CH_DATA* tmpinfo = g_slist_nth_data(this->metadata->prog_info, i);
	if (!(tmpinfo->to_be_analyzed))
	  continue;
	newbranch = ats_branch_new(this->metadata->stream_id,
				   tmpinfo->number,
				   tmpinfo->xid,
				   0,
				   this->metadata);

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
ats_tree_remove_branches(ATS_TREE* this)
{
  guint blen;
  if (this->branches != NULL){
    GstPad *src;
    GstEvent *event;
    src = gst_element_get_static_pad(this->source, "src");
    event = gst_event_new_eos();
    gst_pad_push_event(src, event);
    gst_object_unref(src);
  }
  ats_metadata_reset(this->metadata);
  gst_element_set_state(this->pipeline, GST_STATE_NULL);
  /* deleting processing branch for each channel */
  blen = g_slist_length(this->branches);
  for (guint i = 0; i < blen; i++) {
    ATS_BRANCH* tmp = g_slist_nth_data(this->branches, i);
    if (tmp->bin != NULL)
      gst_bin_remove(GST_BIN(this->pipeline), tmp->bin);
    tmp->bin = NULL;
    ats_branch_delete(tmp);
  }
  /* deleting branches object and channel tee */
  g_slist_free(this->branches);
  this->branches = NULL;
  gst_bin_remove(GST_BIN(this->pipeline), this->tee);
  this->tee = NULL;
  gst_element_set_state(this->pipeline, GST_STATE_READY);
  gst_element_set_state(this->pipeline, GST_STATE_PLAYING);
}

