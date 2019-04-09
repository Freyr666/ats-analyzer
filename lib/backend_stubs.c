#include <caml/bigarray.h>
#include <caml/mlvalues.h>
#include <caml/alloc.h>
#include <caml/threads.h>
#include <caml/fail.h>
#include <caml/custom.h>
#include <caml/memory.h>
#include <caml/callback.h>
#include <caml/threads.h>

typedef void Context;

extern void qoe_backend_init_logger (void);

extern Context* qoe_backend_create (char** error);

extern void qoe_backend_run (Context*);

extern void qoe_backend_quit (Context*);

extern void qoe_backend_free (Context*);

extern char* qoe_backend_get (Context*);

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
caml_qoe_backend_create (value unit) {
        CAMLparam0 ();
        CAMLlocal1 (res);

        Context *context = NULL;
        char *error = NULL;

        caml_release_runtime_system ();
        
        context = qoe_backend_create (&error);

        caml_acquire_runtime_system ();

        printf ("Context received at %p\n", context);
        
        if (context == NULL) {

                if (error) {

                        value err = caml_copy_string (error);
                        free (error);
                        caml_failwith_value (err);
                        
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

        printf ("Context received at %p\n", back);

        caml_release_runtime_system ();

        qoe_backend_run (back);

        caml_acquire_runtime_system ();
        
        CAMLreturn (Val_unit);
}

CAMLprim value
caml_qoe_backend_quit (value backend) {
        CAMLparam1 (backend);
        //CAMLlocal0 ();

        Context * back = Context_val (backend);

        caml_release_runtime_system ();

        qoe_backend_quit (back);

        caml_acquire_runtime_system ();
        
        CAMLreturn(Val_unit);
}

CAMLprim value
caml_qoe_backend_free (value backend) {
        CAMLparam1 (backend);
        //CAMLlocal0 ();
        // TODO properly deallocate
        Context * back = Context_val (backend);

        caml_release_runtime_system ();

        qoe_backend_free (back);

        caml_acquire_runtime_system ();
        
        CAMLreturn(Val_unit);
}

CAMLprim value
caml_qoe_backend_get_streams (value backend) {
        CAMLparam1 (backend);
        CAMLlocal1 (res);
        // TODO properly deallocate
        Context * back = Context_val (backend);
        char* streams;

        printf ("Context received at %p\n", back);
        
        caml_release_runtime_system ();

        streams = qoe_backend_get (back);

        caml_acquire_runtime_system ();
        // TODO reduce allocations
        res = caml_copy_string(streams);
        
        CAMLreturn(res);
}
