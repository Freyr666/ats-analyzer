#include <caml/bigarray.h>
#include <caml/mlvalues.h>
#include <caml/alloc.h>
#include <caml/threads.h>
#include <caml/fail.h>
#include <caml/custom.h>
#include <caml/memory.h>
#include <caml/callback.h>
#include <caml/threads.h>

#include <stdint.h>
#include <stdbool.h>

#include "gstbuffer_stubs.h"
/* 
 * TODO import types from plugins
 */

enum {
  BLACK,
  LUMA,
  FREEZE,
  DIFF,
  BLOCKY,
  VIDEO_ERR_NUM
} VIDEO_ERR;

enum {
  SILENCE_SHORTT,
  SILENCE_MOMENT,
  LOUDNESS_SHORTT,
  LOUDNESS_MOMENT,
  AUDIO_ERR_NUM
} AUDIO_ERR;

enum {
  SHORTT,
  MOMENT,
  AUDIO_MEAS_NUM
} AUDIO_MEAS;

struct point {
  gint64 time;
  double data;
};

struct flag {
  gboolean value;
  gint64   timestamp;
  gint64   span;
};

struct flags {
  struct flag cont_flag;
  struct flag peak_flag;
};

struct data {
  guint32      length;
  guint32      meaningful;
  struct point values [];
};

static inline void
ms_to_days_ps (int64_t ms, int * days, int64_t * ps)
{
  int64_t day = 86400000000;
  int64_t ps_in_ms = 1000000;

  *days = ms / day;
  *ps = (ms % day) * ps_in_ms;
}

value
ms_to_time (int64_t ms)
{
  CAMLparam0 ();
  CAMLlocal2 (caml_ps, rval);

  int64_t ps;
  int days;

  ms_to_days_ps (ms, &days, &ps);

  caml_ps = caml_copy_int64 (ps);

  rval = caml_alloc_tuple (2);

  Store_field (rval, 0, Val_int (days));
  Store_field (rval, 1, caml_ps);
  
  CAMLreturn (rval);
}
  
  
CAMLprim value
caml_video_errors_of_ba (value buf) {
  CAMLparam1 (buf);
  CAMLlocal4 (rval, errors, data, flag);
  CAMLlocal3 (tuple, tmp, array);

  GstMapInfo info;
  GstBuffer  *b = Buffer_val(buf);

  int            good;
  int            days;
  int64_t        ps;
  struct flags * flags;
  struct data *  points;
  void          *ptr, *endbuf;
  //struct error32 * errors;

  caml_release_runtime_system ();
  good = gst_buffer_map (b, &info, GST_MAP_READ);
  caml_acquire_runtime_system ();

  if (! good)
    caml_failwith ("Couldn't map the buffer");

  if (info.size < sizeof (struct flags) * VIDEO_ERR_NUM)
    goto failure;

  flags = (struct flags *) info.data;

  rval   = caml_alloc_tuple (2);
  errors = caml_alloc_tuple (VIDEO_ERR_NUM);
        
  for (int p = 0; p < VIDEO_ERR_NUM; p++)
    {
      tuple = caml_alloc_tuple (2);

      /* Cont error flag */
      flag = caml_alloc_tuple (3);
      /* value */
      Store_field (flag, 0, Val_bool (flags[p].cont_flag.value));
      /* timestamp */
      tmp = ms_to_time (flags[p].cont_flag.timestamp);
      Store_field (flag, 1, tmp);
      /* span */
      tmp = ms_to_time (flags[p].cont_flag.span);
      Store_field (flag, 2, tmp);

      Store_field (tuple, 0, flag);

      /* Peak error flag */
      flag = caml_alloc_tuple (3);
      /* value */
      Store_field (flag, 0, Val_bool (flags[p].peak_flag.value));
      /* timestamp */
      tmp = ms_to_time (flags[p].peak_flag.timestamp);
      Store_field (flag, 1, tmp);
      /* span */
      tmp = ms_to_time (flags[p].peak_flag.span);
      Store_field (flag, 2, tmp);

      Store_field (tuple, 1, flag);

      /* Store error */
      Store_field (errors, p, tuple);
    }

  ptr = info.data + sizeof (struct flags) * VIDEO_ERR_NUM;
  endbuf = info.data + info.size;

  data = caml_alloc_tuple (VIDEO_ERR_NUM);
  
  for (int p = 0; p < VIDEO_ERR_NUM; p++)
    {
      if (ptr >= endbuf)
        goto failure;

      points = (struct data *) ptr;

      if (points->meaningful <= 0)
        {
          array = Atom (0);
        }
      else
        {
          array = caml_alloc (points->meaningful, 0);

          for (int i = 0; i < points->meaningful; i++)
            {
              tuple = caml_alloc_tuple (2);
              /* Time */
              tmp = ms_to_time (points->values[i].time);
              Store_field (tuple, 0, tmp);
              /* Data */
              tmp = caml_copy_double (points->values[i].data);
              Store_field (tuple, 1, tmp);

              Store_field (array, i, tuple);
            }
        }
      
      Store_field (data, p, array);

      ptr += sizeof (struct data) + sizeof (struct point) * points->length;
    }

  Store_field (rval, 0, errors);
  Store_field (rval, 1, data);

  caml_release_runtime_system ();
  gst_buffer_unmap (b, &info);
  caml_acquire_runtime_system ();

  CAMLreturn(rval);

 failure:
  {
    caml_release_runtime_system ();
    gst_buffer_unmap (b, &info);
    caml_acquire_runtime_system ();
    caml_failwith ("Array is smaller than expected");
  }
}

CAMLprim value
caml_audio_errors_of_ba (value buf) {
  CAMLparam1 (buf);
  CAMLlocal4 (rval, errors, data, flag);
  CAMLlocal3 (tuple, tmp, array);

  GstMapInfo info;
  GstBuffer  *b = Buffer_val(buf);

  int            good;
  int            days;
  int64_t        ps;
  struct flags * flags;
  struct data *  points;
  void          *ptr, *endbuf;
  //struct error32 * errors;

  caml_release_runtime_system ();
  good = gst_buffer_map (b, &info, GST_MAP_READ);
  caml_acquire_runtime_system ();

  if (! good)
    caml_failwith ("Couldn't map the buffer");

  if (info.size < sizeof (struct flags) * AUDIO_ERR_NUM)
    goto failure;

  flags = (struct flags *) info.data;

  rval   = caml_alloc_tuple (2);
  errors = caml_alloc_tuple (AUDIO_ERR_NUM);
        
  for (int p = 0; p < AUDIO_ERR_NUM; p++)
    {
      tuple = caml_alloc_tuple (2);

      /* Cont error flag */
      flag = caml_alloc_tuple (3);
      /* value */
      Store_field (flag, 0, Val_bool (flags[p].cont_flag.value));
      /* timestamp */
      tmp = ms_to_time (flags[p].cont_flag.timestamp);
      Store_field (flag, 1, tmp);
      /* span */
      tmp = ms_to_time (flags[p].cont_flag.span);
      Store_field (flag, 2, tmp);

      Store_field (tuple, 0, flag);

      /* Peak error flag */
      flag = caml_alloc_tuple (3);
      /* value */
      Store_field (flag, 0, Val_bool (flags[p].peak_flag.value));
      /* timestamp */
      tmp = ms_to_time (flags[p].peak_flag.timestamp);
      Store_field (flag, 1, tmp);
      /* span */
      tmp = ms_to_time (flags[p].peak_flag.span);
      Store_field (flag, 2, tmp);

      Store_field (tuple, 1, flag);

      /* Store error */
      Store_field (errors, p, tuple);
    }

  ptr = info.data + sizeof (struct flags) * AUDIO_ERR_NUM;
  endbuf = info.data + info.size;

  data = caml_alloc_tuple (AUDIO_MEAS_NUM);
  
  for (int p = 0; p < AUDIO_MEAS_NUM; p++)
    {
      if (ptr >= endbuf)
        goto failure;

      points = (struct data *) ptr;

      if (points->meaningful <= 0)
        {
          array = Atom (0);
        }
      else
        {
          array = caml_alloc (points->meaningful, 0);

          for (int i = 0; i < points->meaningful; i++)
            {
              tuple = caml_alloc_tuple (2);
              /* Time */
              tmp = ms_to_time (points->values[i].time);
              Store_field (tuple, 0, tmp);
              /* Data */
              tmp = caml_copy_double (points->values[i].data);
              Store_field (tuple, 1, tmp);

              Store_field (array, i, tuple);
            }
        }
      
      Store_field (data, p, array);

      ptr += sizeof (struct data) + sizeof (struct point) * points->length;
    }

  Store_field (rval, 0, errors);
  Store_field (rval, 1, data);

  caml_release_runtime_system ();
  gst_buffer_unmap (b, &info);
  caml_acquire_runtime_system ();

  CAMLreturn(rval);

 failure:
  {
    caml_release_runtime_system ();
    gst_buffer_unmap (b, &info);
    caml_acquire_runtime_system ();
    caml_failwith ("Array is smaller than expected");
  }
}

/*
  CAMLprim value
  caml_audio_errors_of_ba (value buf) {
  CAMLparam1 (buf);
  CAMLlocal4 (res, tup, time, pars);
  CAMLlocal3 (min, max, avg);

  int        good;
  GstMapInfo info;
  GstBuffer  *b = Buffer_val(buf);
  struct error64 * errors;

  caml_release_runtime_system ();
  good = gst_buffer_map (b, &info, GST_MAP_READ);
  caml_acquire_runtime_system ();

  if (! good) {
  caml_failwith ("Couldn't map the buffer");
  }

  if (info.size < sizeof (struct error64) * AUDIO_ERR_NUM) {
  caml_release_runtime_system ();
  gst_buffer_unmap (b, &info);
  caml_acquire_runtime_system ();
  caml_failwith ("Array is smaller than expected");
  }

  errors = (struct error64 *) info.data;

  res = caml_alloc_tuple (AUDIO_ERR_NUM);
        
  for (int p = 0; p < AUDIO_ERR_NUM; p++) {
  tup  = caml_alloc_tuple (6);
  pars = caml_alloc_tuple (3);
  min  = caml_copy_double (errors[p].params.min);
  max  = caml_copy_double (errors[p].params.max);
  avg  = caml_copy_double (errors[p].params.avg);
  time = caml_copy_int64 (errors[p].timestamp);
                
  Field(pars, 0) = min;
  Field(pars, 1) = max;
  Field(pars, 2) = max;

  Field(tup, 0) = Val_long (errors[p].counter);
  Field(tup, 1) = Val_long (errors[p].size);
  Field(tup, 2) = pars;
  Field(tup, 3) = time;
  Field(tup, 4) = Val_bool(errors[p].peak_flag);
  Field(tup, 5) = Val_bool(errors[p].cont_flag);

  Field(res, p) = tup;
  }

  caml_release_runtime_system ();
  gst_buffer_unmap (b, &info);
  caml_acquire_runtime_system ();

  CAMLreturn(res);
  }

*/

/*
  struct param32 {
  float min;
  float max;
  float avg;
  };

  struct param64 {
  double min;
  double max;
  double avg;
  };

  struct error32 {
  uint32_t counter;
  uint32_t size;
  struct param32 params;
  int64_t  timestamp;
  bool     peak_flag;
  bool     cont_flag;
  };

  struct error64 {
  uint32_t counter;
  uint32_t size;
  struct param64 params;
  int64_t  timestamp;
  bool     peak_flag;
  bool     cont_flag;
  };
*/
