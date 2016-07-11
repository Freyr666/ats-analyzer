#ifndef ATS_TREE_H
#define ATS_TREE_H

#include <glib.h>
#include <gst/gst.h>

#include "ats_branch.h"
#include "ats_metadata.h"
#include "ats_subbranch.h"

typedef struct __fake_tee
{
  GstElement *tee;
  GstPad *pad;
} FAKE_TEE;

/*
 * Proc_tree type describes tree structured graph
 * for gstreamer based mpeg ts analysis software
 * It consists of three separate layers:
 * ------------------------------
 * Source layer
 * Gst source element
 * |
 * ------------------------------
 * Demux layer
 * Gst mpeg ts demultiplexer
 * |
 * _ _    _
 * | | .. |
 * ------------------------------
 * Processing layer
 * List of processing branches for each audio or video stream
 * | |    |
 * * * .. * 
 *
 * Graph representation
 * source-(middleware)------faketee.tee-------fakesink 
 *        |         |                \-(faketee.pad)--tee--branches.bin(0)
 *        |         \------\                             |-branches.bin(1)
 *        \-queue2-tsparse-/                             ...
 *                                                       \-branches.bin(n)
 * |--Source layer--------------------|--Demux layer-----|-Processing layer-| 
 *
 */

typedef
struct __ats_tree
{
  /* TS metadata */
  ATS_METADATA *metadata;
  /* Elements */
  GstElement *source;
  FAKE_TEE faketee; 
  GstElement *tee;
  GSList *branches;
  /* State */
  GstElement *pipeline;
}ATS_TREE;

ATS_TREE* ats_tree_new(guint stream_id,
		       gchar* ip,
		       guint port,
		       GError** error);

void ats_tree_delete(ATS_TREE* this);

GstBus* ats_tree_get_bus(ATS_TREE* this);

void ats_tree_set_state(ATS_TREE* this,
			GstState state);

ATS_SUBBRANCH* ats_tree_find_subbranch(ATS_TREE* this,
				       guint prog,
				       guint pid);

void ats_tree_add_branches(ATS_TREE* this);

void ats_tree_remove_branches(ATS_TREE* this);

#endif /* ATS_TREE_H */
