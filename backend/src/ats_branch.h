#ifndef ATS_BRANCH_H
#define ATS_BRANCH_H

#include <gst/gst.h>
#include <glib.h>

#include "ats_metadata.h"

typedef struct __ats_subbranch
{
  guint pid;
  gchar* type;
  gchar av;
  GstElement* analyser;
  GstElement* sink;
  GstElement* bin;
} ATS_SUBBRANCH;

typedef struct __ats_branch
{
  guint stream_id;
  guint prog_num;
  /* Branch pipeline*/
  GstElement* bin;
  /* Subbranches */
  GSList* subbranches;
} ATS_BRANCH;

ATS_BRANCH* ats_branch_new(const guint stream_id,
			   const guint prog_num,
			   const guint xid,
			   const double volume,
			   ATS_METADATA* data);

void ats_branch_delete(ATS_BRANCH *this);


#endif /* ATS_BRANCH_H */

