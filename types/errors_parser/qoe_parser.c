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

enum { BLACK, LUMA, FREEZE, DIFF, BLOCKY, VIDEO_ERR_NUM } VIDEO_ERR;
enum { SILENCE_SHORTT, LOUDNESS_SHORTT, SILENCE_MOMENT, LOUDNESS_MOMENT, AUDIO_ERR_NUM } AUDIO_ERR;

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

CAMLprim value
caml_video_errors_of_ba (value buf) {
        CAMLparam1 (buf);
        CAMLlocal4 (res, tup, time, pars);
        CAMLlocal3 (min, max, avg);

        int        good;
        GstMapInfo info;
        GstBuffer  *b = Buffer_val(buf);
        struct error32 * errors;

        caml_release_runtime_system ();
        good = gst_buffer_map (b, &info, GST_MAP_READ);
        caml_acquire_runtime_system ();

        if (! good) {
                caml_failwith ("Couldn't map the buffer");
        }

        if (info.size < sizeof (struct error32) * VIDEO_ERR_NUM) {
                caml_release_runtime_system ();
                gst_buffer_unmap (b, &info);
                caml_acquire_runtime_system ();
                caml_failwith ("Array is smaller than expected");
        }

        errors = (struct error32 *) info.data;

        res = caml_alloc_tuple (VIDEO_ERR_NUM);
        
        for (int p = 0; p < VIDEO_ERR_NUM; p++) {
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
