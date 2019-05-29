#include "gstbuffer_stubs.h"

static void
buffer_finalize(value buf) {
        GstBuffer *b = Buffer_val(buf);
        if (b) {
                gst_buffer_unref(b);
        }
}

static struct custom_operations buffer_ops = {
        "gstbuffer",
        buffer_finalize,
        custom_compare_default,
        custom_hash_default,
        custom_serialize_default,
        custom_deserialize_default
};

CAMLexport value
caml_gstbuffer_alloc (GstBuffer *b) {
        CAMLparam0 ();
        CAMLlocal1 (v);
        v = alloc_custom(&buffer_ops, sizeof(GstBuffer *), 0, 1);
        Buffer_val(v) = b;
        CAMLreturn(v);
}

CAMLprim value
caml_gstbuffer_process (value buf,
                        value proc) {
        CAMLparam2 (buf, proc);
        CAMLlocal2 (ba, res);

        long dims[1];
        GstMapInfo info;
        GstBuffer *b = Buffer_val(buf);
        
        // caml_release_runtime_system ();

        if (! gst_buffer_map (b, &info, GST_MAP_READ)) {
                //  caml_acquire_runtime_system ();
                // goto error;
                caml_failwith ("Couldn't map the buffer");
        }

        // caml_acquire_runtime_system ();

        dims[0] = info.size;
        ba = caml_ba_alloc (CAML_BA_CHAR | CAML_BA_C_LAYOUT | CAML_BA_EXTERNAL,
                            1, /* 1d array */
                            info.data,
                            dims);

        res = caml_callback (proc, ba);

        gst_buffer_unmap (b, &info);

        CAMLreturn(res);
//error:
//        caml_failwith ("Couldn't map the buffer");
}
