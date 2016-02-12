#ifndef ATS_BRANCH_H
#define ATS_BRANCH_H

#include <gst/gst.h>
#include <glib.h>

#include "ats_metadata.h"

typedef
struct __ats_branch
{
  guint stream_id;
  guint prog_num;
  guint xid;
  /* Branch pipeline*/
  GstElement* bin;
} ATS_BRANCH;

ATS_BRANCH* ats_branch_new(const guint stream_id,
			     const guint prog_num,
			     const guint xid);

void ats_branch_delete(ATS_BRANCH *this);


#endif /* ATS_BRANCH_H */

