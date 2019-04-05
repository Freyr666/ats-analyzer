#include <caml/bigarray.h>
#include <caml/mlvalues.h>
#include <caml/alloc.h>
#include <caml/threads.h>
#include <caml/fail.h>
#include <caml/custom.h>
#include <caml/memory.h>
#include <caml/callback.h>
#include <caml/threads.h>

typedef void* Context_ptr;

extern void qoe_backend_init_logger (void);

extern Context_ptr qoe_backend_create (char** error);

static struct custom_operations context_ops = {
  "context",
  custom_finalize_default,
  custom_compare_default,
  custom_hash_default,
  custom_serialize_default,
  custom_deserialize_default,
  custom_compare_ext_default
};

#define Context_val(v) (*((Context_ptr *) Data_custom_val(v)))

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

        Context_ptr context = NULL;
        char* error = NULL;

        caml_release_runtime_system ();
        
        context = qoe_backend_create (&error);

        caml_acquire_runtime_system ();

        if (context == NULL) {

                if (error) {

                        value err = caml_copy_string (error);
                        free (error);
                        caml_failwith_value (err);
                        
                } else {

                        caml_failwith ("Unknown error");
                        
                }

                res = alloc_custom (&context_ops, sizeof(Context_ptr), 0, 1);
                
        }

        CAMLreturn(res);
}
