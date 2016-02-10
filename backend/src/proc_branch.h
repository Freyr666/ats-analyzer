#ifndef PROC_BRANCH_H
#define PROC_BRANCH_H

#include <gst/gst.h>
#include <glib.h>

#include "proc_metadata.h"

typedef
struct __proc_branch
{
  guint stream_id;
  guint prog_num;
  guint xid;
  /* Branch pipeline*/
  GstElement* bin;
} PROC_BRANCH;

PROC_BRANCH* proc_branch_new(const guint stream_id,
			     const guint prog_num,
			     const guint xid);

void proc_branch_delete(PROC_BRANCH *this);


#endif /* PROC_BRANCH_H */

