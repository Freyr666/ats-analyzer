#ifndef ATS_CONTROL_H
#define ATS_CONTROL_H

#include <glib.h>
#include <gio/gio.h>
#include "ats_tree.h"

#define TREE_HEADER 0xDEADBEEF
#define STREAM_DIVIDER 0xABBA0000
#define PROG_DIVIDER 0xACDC0000

#define SOUND_HEADER 0x0EFA1922

#define VIDEO_SETTINGS_HEADER 0xCDDA1307
#define SETTINGS_BLACK_LEVEL 0xBA1306BA
#define SETTINGS_DIFF_LEVEL 0xDA0476AD
#define SETTINGS_MARK_BLOCKS 0xCA12AD86

typedef struct __ats_control
{
  /* input */
  GSocketService * incoming_service;
  /* output */
  GSocketClient * client;
} ATS_CONTROL;

ATS_CONTROL* ats_control_init(ATS_CONTROL* ctrl,
			      ATS_TREE* tree,
			      guint stream_id,
			      GError** error);

ATS_CONTROL* ats_control_new(ATS_TREE* tree,
			     guint stream_id,
			     GError** error);

void ats_control_delete(ATS_CONTROL* this);

void ats_control_send(ATS_CONTROL* this,
		      gchar* message,
		      GError** error);

#endif /* ATS_CONTROL_H */
