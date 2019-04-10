#include <caml/bigarray.h>
#include <caml/mlvalues.h>
#include <caml/alloc.h>
#include <caml/threads.h>
#include <caml/fail.h>
#include <caml/custom.h>
#include <caml/memory.h>
#include <caml/callback.h>
#include <caml/threads.h>

#include "qoebackend.h"

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
caml_qoe_backend_create (value array) {
        CAMLparam1 (array);
        CAMLlocal2 (tmp,res);

        Context  *context = NULL;
        char     *error = NULL;
        uint32_t size = caml_array_length(array);
        struct init_val * args = (struct init_val *)malloc(sizeof(struct init_val) * size);

        for (int i = 0; i < size; i++) {
                tmp = Field(array,i);
                args[i].tag = caml_stat_strdup(String_val(Field(tmp,0)));
                args[i].arg1 = caml_stat_strdup(String_val(Field(tmp,1)));
        }

        caml_release_runtime_system ();
        
        context = qoe_backend_create (args, size, &error);

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

        res = alloc_custom (&context_ops, sizeof(Context*), 0, 1);
        Context_val(res) = context;

        CAMLreturn(res);
}

CAMLprim value
caml_qoe_backend_run (value backend) {
        CAMLparam1 (backend);
        //CAMLlocal0 ();

        Context * back = Context_val (backend);

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
        Context * back = Context_val (backend);

        if (back == NULL)
                caml_failwith ("Invalid context");

        caml_release_runtime_system ();

        qoe_backend_free (back);

        caml_acquire_runtime_system ();

        Context_val (backend) = NULL;
        
        CAMLreturn(Val_unit);
}

CAMLprim value
caml_qoe_backend_stream_parser_get_structure (value backend) {
        CAMLparam1 (backend);
        CAMLlocal1 (res);
        // TODO properly deallocate
        Context * back = Context_val (backend);
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
        Context * back = Context_val (backend);
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
        Context *  back = Context_val (backend);
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
        Context * back = Context_val (backend);
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
        Context *  back = Context_val (backend);
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
