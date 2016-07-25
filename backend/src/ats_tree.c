#include "ats_tree.h"
#include "unistd.h"

ATS_TREE*
ats_tree_new(guint    stream_id,
	     gchar*   ip,
	     guint    port,
	     GError** error)
{
  ATS_TREE       *rval;
  GstElement     *parse, *fakesink;
  GstPadTemplate *tee_src_pad_template;
  GstPad         *teepad, *sinkpad;
  
  /* init-ing tree */
  rval              = g_new(ATS_TREE, 1);
  rval->pipeline    = NULL;
  rval->source      = NULL;
  parse             = NULL;
  fakesink          = NULL;
  rval->faketee.tee = NULL;
  rval->faketee.pad = NULL;
  rval->branches    = NULL;
  
  /* creating elements */
  if ((rval->pipeline = gst_pipeline_new("proc-tree-pipe")) == NULL)
    goto error;
  
  if ((rval->source = gst_element_factory_make("udpsrc","proc-tree-source")) == NULL)
    goto error;
  
  if ((parse = gst_element_factory_make("tsparse","proc-tree-parse")) == NULL)
    goto error;
  
  if ((rval->faketee.tee = gst_element_factory_make("tee","proc-tree-tee")) == NULL)
    goto error;
  
  if ((fakesink = gst_element_factory_make("fakesink",NULL)) == NULL)
    goto error;
  
  /* init-ing tree metadata */
  rval->metadata = ats_metadata_new(stream_id);
  
  /* setting udpsrc port and buf size*/
  g_object_set (G_OBJECT (rval->source),
		"timeout",     5000000000,
		"buffer-size", 2147483647,
		"port",        port,
		"address",     ip,
		NULL);

  g_object_set(G_OBJECT(parse),
	       "parse-private-sections", TRUE,
	       NULL);
  
  /* linking pipeline */
  gst_bin_add_many(GST_BIN(rval->pipeline),
		   rval->source,
		   parse,
		   rval->faketee.tee,
		   fakesink, NULL);
  
  gst_element_link_many (rval->source,
			 parse,
			 rval->faketee.tee, NULL);

  /* connecting tee src to fakesink */
  sinkpad = gst_element_get_static_pad(fakesink, "sink");
  tee_src_pad_template =
    gst_element_class_get_pad_template(GST_ELEMENT_GET_CLASS
				       (rval->faketee.tee),
				       "src_%u");
  teepad = gst_element_request_pad(rval->faketee.tee,
				   tee_src_pad_template,
				   NULL, NULL);
  gst_pad_link(teepad, sinkpad);
  gst_object_unref(tee_src_pad_template);
  gst_object_unref(teepad);

  /* creating additional tee src pad for other brunches */
  tee_src_pad_template =
    gst_element_class_get_pad_template(GST_ELEMENT_GET_CLASS(rval->faketee.tee),
				       "src_%u");
  rval->faketee.pad = gst_element_request_pad(rval->faketee.tee,
					      tee_src_pad_template,
					      NULL, NULL);
  gst_object_unref(tee_src_pad_template);
  
  return rval;
 error:
  if (rval->pipeline)    gst_object_unref(rval->pipeline);
  if (rval->source)      gst_object_unref(rval->source);
  if (parse)             gst_object_unref(parse);
  if (rval->faketee.tee) gst_object_unref(rval->faketee.tee);
  if (fakesink)          gst_object_unref(fakesink);
  if (rval)              g_free(rval);
  g_set_error(error,
	      G_ERR_UNKNOWN, -1,
	      "Error: failed to create tree in ats_tree_new");
  return NULL;
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
			guint     prog,
			guint     pid)
{
  GSList*          elem_ch;
  GSList*          elem_pid;
  ATS_BRANCH*      branch;
  ATS_SUBBRANCH*   subbranch;
  
  if(this->branches == NULL) return NULL;

  for (elem_ch = this->branches; elem_ch; elem_ch = elem_ch->next) {
    branch = elem_ch->data;

    if (branch->prog_num != prog) continue;
    if (branch->subbranches == NULL) return NULL;

    for (elem_pid = branch->subbranches; elem_pid; elem_pid = elem_pid->next) {
      subbranch = elem_pid->data;

      if (subbranch->pid != pid) continue;
      return subbranch;
    }
    return NULL;
  }
  return NULL;
}

void
ats_tree_add_branches(ATS_TREE* this,
		      GError**  error)
{
  ATS_BRANCH     *newbranch;
  GstPadTemplate *tee_src_pad_template;
  GstPad         *teepad, *branchpad, *sinkpad;
  ATS_CH_DATA    *ch_data;
  GSList         *meta_elem;
  GError         *local_error;

  local_error = NULL;
  gst_element_set_state(this->pipeline, GST_STATE_PAUSED);
  
  /* if TS have been parsed */
  if (this->metadata->prog_info != NULL){
    
    if(this->faketee.pad == NULL){
      g_set_error(error,
		  G_ERR_UNKNOWN, -1,
		  "Error: ats_tree_add_branches: tee has no pad to connect");
      return;
    }
    
    /* conecting channel tee pad to the graph */
    this->tee = gst_element_factory_make("tee", NULL);
    gst_bin_add(GST_BIN(this->pipeline), this->tee);
    sinkpad = gst_element_get_static_pad(this->tee, "sink");
    gst_pad_link(this->faketee.pad, sinkpad);

    /* adding processing branch for each channel */
    for (meta_elem = this->metadata->prog_info; meta_elem; meta_elem = meta_elem->next) {
      ch_data = meta_elem->data;
      
      if (!(ch_data->to_be_analyzed)) continue;
      
      newbranch = ats_branch_new(this->metadata->stream_id,
				 ch_data->number,
				 ch_data->xid,
				 0,
				 this->metadata,
				 &local_error);

      if (newbranch == NULL) {
	g_propagate_error(error, local_error);
	goto error;
      }

      this->branches = g_slist_append(this->branches, newbranch);

      gst_bin_add(GST_BIN(this->pipeline), newbranch->bin);
      gst_bin_sync_children_states(GST_BIN(this->pipeline));

      /* connecting processing branch to channel tee */
      branchpad = gst_element_get_static_pad(newbranch->bin, "sink");
      tee_src_pad_template =
	gst_element_class_get_pad_template (GST_ELEMENT_GET_CLASS (this->tee),
					    "src_%u");
      teepad = gst_element_request_pad (this->tee, tee_src_pad_template, NULL, NULL);
      gst_pad_link(teepad, branchpad);
      gst_object_unref (teepad);
      gst_object_unref (branchpad);
    }
  }
  gst_element_set_state(this->pipeline, GST_STATE_PLAYING);
  /* g_printerr("ats_tree is in PLAYING state!\n"); */
  return;
 error:
  if (this->tee)    gst_object_unref(this->tee);
  ats_tree_remove_branches(this);
  return;
}

void
ats_tree_remove_branches(ATS_TREE* this)
{
  GstPad*       src;
  GstEvent*     event;
  GSList*       elem;
  ATS_BRANCH*   branch;

  /* Push EOS event into the pipeline */
  if (this->branches != NULL){
    src = gst_element_get_static_pad(this->source, "src");
    event = gst_event_new_eos();
    gst_pad_push_event(src, event);
    gst_object_unref(src);
  }
  
  ats_metadata_reset(this->metadata);
  gst_element_set_state(this->pipeline, GST_STATE_NULL);

  /* deleting processing branch for each channel */
  for (elem = this->branches; elem; elem = elem->next) {
    branch = elem->data;

    if (branch->bin) gst_bin_remove(GST_BIN(this->pipeline), branch->bin);
    branch->bin = NULL;
    ats_branch_delete(branch);
  }

  /* deleting branches object and channel tee */
  g_slist_free(this->branches);
  this->branches = NULL;
  gst_bin_remove(GST_BIN(this->pipeline), this->tee);
  this->tee = NULL;
  gst_element_set_state(this->pipeline, GST_STATE_READY);
  gst_element_set_state(this->pipeline, GST_STATE_PLAYING);
}

