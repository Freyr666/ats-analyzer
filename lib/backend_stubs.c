#include <caml/bigarray.h>
#include <caml/mlvalues.h>
#include <caml/alloc.h>
#include <caml/threads.h>
#include <caml/fail.h>
#include <caml/custom.h>
#include <caml/memory.h>
#include <caml/callback.h>
#include <caml/threads.h>

#include <stdatomic.h>
#include <stdbool.h>

#include "qoebackend.h"

atomic_bool running = false;

value * streams_closure = NULL;

void streams_thread_register (void) {
        caml_c_thread_register();
}

void streams_thread_unregister (void) {
        caml_c_thread_unregister();
}

void streams_callback (char* s) {
        CAMLparam0 ();
        CAMLlocal1 (arg);

        /* TODO add streams_thread_register cb
         * caml_c_thread_register();
         */

        caml_acquire_runtime_system ();

        arg = caml_copy_string(s);
        caml_callback (*streams_closure, arg);

        caml_release_runtime_system();

        /* TODO add streams_thread_unregister cb
         * caml_c_thread_unregister();
         */
        
        free(s);
        CAMLreturn0;
}

static struct custom_operations context_ops = {
  "context",
  custom_finalize_default,
  custom_compare_default,
  custom_hash_default,
  custom_serialize_default,
  custom_deserialize_default,
  custom_compare_ext_default
};

#define Context_val(v) (*((Context **) Data_custom_val(v)))

CAMLprim value
caml_qoe_backend_init_logger (value unit) {
        CAMLparam0 ();

        qoe_backend_init_logger ();

        CAMLreturn(unit);
}

CAMLprim value
caml_qoe_backend_create (value array,
                         value streams_cb) {
        CAMLparam2 (array, streams_cb);
        CAMLlocal3 (tmp, ctx, res);

        Context  *context = NULL;
        char     *error = NULL;
        uint32_t size;
        struct init_val * args;

        if (atomic_load (&running) == true) {
                caml_failwith ("Other pipeline instance is already running");
        }

        atomic_store (&running, true);

        size = caml_array_length(array);
        args = (struct init_val *)malloc(sizeof(struct init_val) * size);

        for (int i = 0; i < size; i++) {
                tmp = Field(array,i);
                args[i].tag = caml_stat_strdup(String_val(Field(tmp,0)));
                args[i].arg1 = caml_stat_strdup(String_val(Field(tmp,1)));
        }

        caml_release_runtime_system ();
        
        context = qoe_backend_create (args, size, streams_callback, &error);

        for (int i = 0; i < size; i++) {
                caml_stat_free (args[i].tag);
                caml_stat_free (args[i].arg1);
        }
        free (args);
        
        caml_acquire_runtime_system ();
        //printf ("Context received at %p\n", context);
        
        if (context == NULL) {
                if (error) {
                        caml_failwith (error);
                        free (error);
                } else {
                        caml_failwith ("Unknown error");
                }
        }

        ctx = alloc_custom (&context_ops, sizeof(Context*), 0, 1);
        Context_val(ctx) = context;

        res = alloc_tuple (2);

        Field(res, 0) = ctx;
        Field(res, 1) = streams_cb;

        streams_closure = &Field(res, 1);
        
        CAMLreturn(res);
}

CAMLprim value
caml_qoe_backend_run (value backend) {
        CAMLparam1 (backend);
        //CAMLlocal0 ();

        Context * back = Context_val (Field (backend, 0));

        if (back == NULL)
                caml_failwith ("Invalid context");
        //printf ("Context received at %p\n", back);

        caml_release_runtime_system ();

        qoe_backend_run (back);

        caml_acquire_runtime_system ();
        
        CAMLreturn (Val_unit);
}

CAMLprim value
caml_qoe_backend_free (value backend) {
        CAMLparam1 (backend);
        //CAMLlocal0 ();
        // TODO properly deallocate
        Context * back = Context_val (Field (backend, 0));

        if (back == NULL)
                caml_failwith ("Invalid context");

        caml_release_runtime_system ();

        qoe_backend_free (back);

        caml_acquire_runtime_system ();

        Context_val (Field (backend, 0)) = NULL;

        streams_closure = NULL;
        
        atomic_store (&running, false);
        
        CAMLreturn(Val_unit);
}

CAMLprim value
caml_qoe_backend_stream_parser_get_structure (value backend) {
        CAMLparam1 (backend);
        CAMLlocal1 (res);
        // TODO properly deallocate
        Context * back = Context_val (Field (backend, 0));
        char    * streams;

        if (back == NULL)
                caml_failwith ("Invalid context");

        caml_release_runtime_system ();

        streams = qoe_backend_stream_parser_get_structure (back);

        caml_acquire_runtime_system ();
        // TODO reduce allocations
        res = caml_copy_string(streams);
        free (streams);
        
        CAMLreturn(res);
}

CAMLprim value
caml_qoe_backend_graph_get_structure (value backend) {
        CAMLparam1 (backend);
        CAMLlocal1 (res);
        // TODO properly deallocate
        Context * back = Context_val (Field (backend, 0));
        char    * streams;

        if (back == NULL)
                caml_failwith ("Invalid context");

        caml_release_runtime_system ();

        streams = qoe_backend_graph_get_structure (back);

        caml_acquire_runtime_system ();
        // TODO reduce allocations
        res = caml_copy_string(streams);
        free (streams);
        
        CAMLreturn(res);
}

CAMLprim value
caml_qoe_backend_graph_apply_structure (value backend,
                                        value streams) {
        CAMLparam1 (backend);
        //CAMLlocal1 ();
        // TODO properly deallocate
        Context *  back = Context_val (Field (backend, 0));
        char    *  streams_str;
        char    ** error = NULL;
        int        res;

        if (back == NULL)
                caml_failwith ("Invalid context");

        streams_str = caml_stat_strdup(String_val(streams));
        
        caml_release_runtime_system ();

        res = qoe_backend_graph_apply_structure (back, streams_str, error);

        caml_stat_free (streams_str);
        
        caml_acquire_runtime_system ();
        // TODO reduce allocations
        if (res != 0) {
                if (error) {
                        caml_failwith (*error);
                        free (*error);
                } else {
                        caml_failwith ("Unknown error");
                }
        }
        
        CAMLreturn(Val_unit);
}

CAMLprim value
caml_qoe_backend_wm_get_layout (value backend) {
        CAMLparam1 (backend);
        CAMLlocal1 (res);
        // TODO properly deallocate
        Context * back = Context_val (Field (backend, 0));
        char    * layout;

        if (back == NULL)
                caml_failwith ("Invalid context");

        caml_release_runtime_system ();

        layout = qoe_backend_wm_get_layout (back);

        caml_acquire_runtime_system ();
        // TODO reduce allocations
        res = caml_copy_string(layout);
        free (layout);
        
        CAMLreturn(res);
}

CAMLprim value
caml_qoe_backend_wm_apply_layout (value backend,
                                  value layout) {
        CAMLparam1 (backend);
        //CAMLlocal1 ();
        // TODO properly deallocate
        Context *  back = Context_val (Field (backend, 0));
        char    *  layout_str;
        char    ** error = NULL;
        int        res;

        if (back == NULL)
                caml_failwith ("Invalid context");

        layout_str = caml_stat_strdup(String_val(layout));
        
        caml_release_runtime_system ();

        res = qoe_backend_wm_apply_layout (back, layout_str, error);

        caml_stat_free (layout_str);
        
        caml_acquire_runtime_system ();
        // TODO reduce allocations
        if (res != 0) {
                if (error) {
                        caml_failwith (*error);
                        free (*error);
                } else {
                        caml_failwith ("Unknown error");
                }
        }
        
        CAMLreturn(Val_unit);
}
