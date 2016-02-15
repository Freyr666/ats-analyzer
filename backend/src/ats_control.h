#ifndef ATS_CONTROL_H
#define ATS_CONTROL_H

#include <glib.h>
#include <gio/gio.h>
#include "ats_tree.h"

#define TREE_HEADER 0xDEADBEEF
#define STREAM_DIVIDER 0xABBA0000
#define PROG_DIVIDER 0xACDC0000

typedef struct __ats_control
{
  GSocketService * incoming_service;
  GSocketClient * client;
} ATS_CONTROL;

ATS_CONTROL* ats_control_new(ATS_TREE* tree);

void ats_control_delete(ATS_CONTROL* this);

void ats_control_send(ATS_CONTROL* this, gchar* message);

#endif /* ATS_CONTROL_H */
