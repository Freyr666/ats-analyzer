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

#include <stdio.h>

#include "gstbuffer_stubs.h"
#include "qoebackend.h"

atomic_bool running = false;

void thread_register (void) {
        printf("Reg thread\n");
        caml_c_thread_register();
}

void thread_unregister (void) {
        printf("Unreg thread\n");
        caml_c_thread_unregister();
}

value streams_closure = 0;

void streams_callback (char* s) {
        CAMLparam0 ();
        CAMLlocal1 (arg);
        
        caml_acquire_runtime_system ();

        arg = caml_copy_string(s);
        caml_callback (streams_closure, arg);

        caml_release_runtime_system();
        
        free(s);
        CAMLreturn0;
}

value graph_closure = 0;

void graph_callback (char* s) {
        CAMLparam0 ();
        CAMLlocal1 (arg);

        caml_acquire_runtime_system ();

        arg = caml_copy_string(s);
        caml_callback (graph_closure, arg);

        caml_release_runtime_system();

        free(s);
        CAMLreturn0;
}

value wm_closure = 0;

void wm_callback (char* s) {
        CAMLparam0 ();
        CAMLlocal1 (arg);

        caml_acquire_runtime_system ();

        arg = caml_copy_string(s);
        caml_callback (wm_closure, arg);

        caml_release_runtime_system();

        free(s);
        CAMLreturn0;
}

value data_closure = 0;

void data_callback (int32_t typ, char* s, uint32_t c, uint32_t p, void* b) {
        CAMLparam0 ();
        CAMLlocal2 (arg, buf);

        value args[4];
        
        caml_acquire_runtime_system ();

        arg = caml_copy_string(s);
        buf = caml_gstbuffer_alloc ((GstBuffer*) b);

        args[0] = Val_int (typ);
        args[1] = arg;
        args[2] = Val_int (c);
        args[3] = Val_int (p);
        args[4] = buf;
        caml_callbackN (data_closure, 5, args);

        caml_release_runtime_system();

        free(s);
        CAMLreturn0;
}

value status_closure = 0;

void status_callback (char* s, uint32_t c, uint32_t p, bool b) {
        CAMLparam0 ();
        CAMLlocal2 (arg, buf);

        value args[4];

        caml_acquire_runtime_system ();

        arg = caml_copy_string(s);
        buf = caml_gstbuffer_alloc ((GstBuffer*) b);

        args[0] = arg;
        args[1] = Val_int (c);
        args[2] = Val_int (p);
        args[3] = Val_bool(b);
        caml_callbackN (status_closure, 4, args);

        caml_release_runtime_system();

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
caml_qoe_backend_create_native  (value array,
                                 value streams_cb,
                                 value graph_cb,
                                 value wm_cb,
                                 value data_cb,
                                 value status_cb) {
        CAMLparam5 (array, streams_cb, graph_cb, wm_cb, data_cb);
        CAMLxparam1 (status_cb);
        CAMLlocal3 (tmp, ctx, res);

        Context  *context = NULL;
        char     *error = NULL;
        uint32_t size;
        struct init_val * args;
        struct callback streams_funs = {
                .cb = streams_callback,
                .reg_thread = thread_register,
                .unreg_thread = thread_unregister,
        };
        struct callback graph_funs = {
                .cb = graph_callback,
                .reg_thread = thread_register,
                .unreg_thread = thread_unregister,
        };
        struct callback wm_funs = {
                .cb = wm_callback,
                .reg_thread = thread_register,
                .unreg_thread = thread_unregister,
        };
        struct data_callback data_funs = {
                .cb = data_callback,
                .reg_thread = thread_register,
                .unreg_thread = thread_unregister,
        };
        struct status_callback status_funs = {
                .cb = status_callback,
                .reg_thread = thread_register,
                .unreg_thread = thread_unregister,
        };

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
        
        res = alloc_tuple (6);

        ctx = alloc_custom (&context_ops, sizeof(Context*), 0, 1);
        caml_initialize(&Field(res, 0), ctx);
        caml_initialize(&Field(res, 1), streams_cb);
        caml_initialize(&Field(res, 2), graph_cb);
        caml_initialize(&Field(res, 3), wm_cb);
        caml_initialize(&Field(res, 4), data_cb);
        caml_initialize(&Field(res, 5), status_cb);
        
        // Save callbacks
        streams_closure = Field(res, 1);
        graph_closure = Field(res, 2);
        wm_closure = Field(res, 3);
        data_closure = Field(res, 4);
        status_closure = Field(res, 5);

        caml_register_global_root(&streams_closure);
        caml_register_global_root(&graph_closure);
        caml_register_global_root(&wm_closure);
        caml_register_global_root(&data_closure);
        caml_register_global_root(&status_closure);

        caml_release_runtime_system ();
        
        context = qoe_backend_create (args,
                                      size,
                                      streams_funs,
                                      graph_funs,
                                      wm_funs,
                                      data_funs,
                                      status_funs,
                                      &error);

        for (int i = 0; i < size; i++) {
                caml_stat_free (args[i].tag);
                caml_stat_free (args[i].arg1);
        }
        free (args);
        
        caml_acquire_runtime_system ();
        //printf ("Context received at %p\n", context);
        // Save the context
        Context_val(Field(res, 0)) = context;
        
        if (context == NULL) {
                if (error) {
                        caml_failwith (error);
                        free (error);
                } else {
                        caml_failwith ("Unknown error");
                }
        }
        
        CAMLreturn(res);
}

CAMLprim value
caml_qoe_backend_create_bytecode  (value * argv,
                                   int argn ) {
        return caml_qoe_backend_create_native (argv[0], argv[1], argv[2],
                                               argv[3], argv[4], argv[5] );
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

        caml_remove_global_root(&streams_closure);
        caml_remove_global_root(&graph_closure);
        caml_remove_global_root(&wm_closure);
        caml_remove_global_root(&data_closure);
        caml_remove_global_root(&status_closure);   

        //streams_closure = NULL;
        
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
