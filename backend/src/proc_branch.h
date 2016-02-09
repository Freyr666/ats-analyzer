#ifndef PROC_BRANCH_H
#define PROC_BRANCH_H

#include <gst/gst.h>
#include <glib.h>

typedef
struct __proc_branch
{
  guint number;
  gchar* service_name;
  gchar* provider_name;
  guint xid;
  /* Branch pipeline*/
  GstElement* bin;
} PROC_BRANCH;

PROC_BRANCH* proc_branch_new(const guint number,
			     const gchar* service_name,
			     const gchar* provider_name,
			     const guint xid);

void proc_branch_delete(PROC_BRANCH *this);


#endif /* PROC_BRANCH_H */

