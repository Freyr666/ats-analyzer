#ifndef ATS_SUBBRANCH_H
#define ATS_SUBBRANCH_H

#include <gst/gst.h>
#include <glib.h>

typedef struct __ats_subbranch
{
  guint         pid;
  gchar*        type;
  gchar         av;
  GstElement*   analyser;
  GstElement*   sink;
  GstElement*   bin;
} ATS_SUBBRANCH;

ATS_SUBBRANCH* create_video_subbranch(const gchar* type,
				      const guint  stream,
				      const guint  prog,
				      const guint  pid,
				      const guint  xid,
				      GError**     error);

ATS_SUBBRANCH* create_audio_subbranch(const gchar* type,
				      const guint  stream,
				      const guint  prog,
				      const guint  pid,
				      const double volume,
				      GError**     error);

void ats_subbranch_delete(ATS_SUBBRANCH* sub);

#endif /* ATS_SUBBRANCH_H */
