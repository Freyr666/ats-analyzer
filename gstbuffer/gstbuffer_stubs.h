#ifndef __CAML_GSTBUFFER__
#define __CAML_GSTBUFFER__

#include <caml/bigarray.h>
#include <caml/mlvalues.h>
#include <caml/alloc.h>
#include <caml/threads.h>
#include <caml/fail.h>
#include <caml/custom.h>
#include <caml/memory.h>
#include <caml/callback.h>
#include <caml/threads.h>

#include <gst/gst.h>

#define Buffer_val(buf) (*(GstBuffer**)Data_custom_val(buf))

CAMLexport value caml_gstbuffer_alloc (GstBuffer *b);
CAMLprim value caml_gstbuffer_process (value buf,
                                       value proc);

#endif /*__CAML_GSTBUFFER__*/
