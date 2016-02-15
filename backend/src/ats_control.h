#ifndef ATS_CONTROL_H
#define ATS_CONTROL_H

#include <glib.h>
#include <gio/gio.h>
#include "ats_tree.h"

typedef struct __ats_control
{
  GSocketService * incoming_service;
  GSocketClient * client;
} ATS_CONTROL;

ATS_CONTROL* ats_control_new(ATS_TREE* tree);

void ats_control_delete(ATS_CONTROL* this);

void ats_control_send(ATS_CONTROL* this, gchar* message);

#endif /* ATS_CONTROL_H */
