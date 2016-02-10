#ifndef PROC_TREE_H
#define PROC_TREE_H

#include <glib.h>
#include <gst/gst.h>

#include "proc_branch.h"
#include "proc_metadata.h"

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
struct __proc_tree
{
  /* TS metadata */
  PROC_METADATA *metadata;
  /* Elements */
  GstElement *source;
  FAKE_TEE faketee; 
  GstElement *tee;
  GSList *branches;
  /* State */
  GstElement *pipeline;
}PROC_TREE;

PROC_TREE* proc_tree_new(guint stream_id);

void proc_tree_delete(PROC_TREE* this);

GstBus* proc_tree_get_bus(PROC_TREE* this);

void proc_tree_set_source(PROC_TREE* this,
			  const gchar* srcpath,
			  const gchar* srcaddress,
			  const guint srcport );

void proc_tree_set_state(PROC_TREE* this,
			 GstState state);

void proc_tree_add_branches(PROC_TREE* this);

void proc_tree_remove_branches(PROC_TREE* this);

#endif /* PROC_TREE_H */
