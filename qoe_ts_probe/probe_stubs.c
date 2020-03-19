#include <caml/bigarray.h>
#include <caml/mlvalues.h>
#include <caml/alloc.h>
#include <caml/threads.h>
#include <caml/fail.h>
#include <caml/custom.h>
#include <caml/memory.h>
#include <caml/callback.h>
#include <caml/threads.h>

//#include <stdatomic.h>
#include <stdbool.h>

#include <stdio.h>
#include <assert.h>

#include "tsprobe.h"

/*
 * A reasonable number of probes able to operate simultaniously
 */
#define MAX_PROBES 512

value closures [MAX_PROBES] = { 0 };

static int32_t
alloc_callback (value v)
{
  int32_t i;
  
  for (i = 0; i < MAX_PROBES; i++)
    {
      if (closures[i] == 0)
        {
          closures[i] = v;
          return i;
        }
    }

  return -1;
}

static void
dealloc_callback (int32_t i)
{
  assert (i >= 0 && i < MAX_PROBES);

  closures[i] = 0;
}
  
void
thread_register (void) {
  printf("Reg thread\n");
  caml_c_thread_register();
}

void
thread_unregister (void) {
  printf("Unreg thread\n");
  caml_c_thread_unregister();
}

void
streams_callback_with_lock (int32_t id, char* s)
{
  CAMLparam0 ();
  CAMLlocal1 (arg);
        
  arg = caml_copy_string(s);
  caml_callback (closures[id], arg);
        
  CAMLreturn0;
}

void
streams_callback (int32_t id, char* s)
{        
  caml_acquire_runtime_system ();

  streams_callback_with_lock (id, s);

  caml_release_runtime_system();

  free(s);
}

static struct custom_operations probe_ops = {
  "probe",
  custom_finalize_default,
  custom_compare_default,
  custom_hash_default,
  custom_serialize_default,
  custom_deserialize_default,
  custom_compare_ext_default
};

#define Probe_val(v) (*((Probe **) Data_custom_val(v)))

CAMLprim value
caml_qoe_probe_create (value tuple,
                       value streams_cb)
{
  CAMLparam2 (tuple, streams_cb);
  CAMLlocal3 (tmp, prb, rval);

  Probe  *probe = NULL;
  char   *error = NULL;
  int32_t index;
  struct init_val args;
  struct callback streams_funs = {
    .id = 0,
    .cb = streams_callback,
    .reg_thread = thread_register,
    .unreg_thread = thread_unregister,
  };

  args.tag = caml_stat_strdup(String_val(Field(tuple,0)));
  args.arg1 = caml_stat_strdup(String_val(Field(tuple,1)));
        
  prb = alloc_custom (&probe_ops, sizeof(Probe*), 0, 1);
        
  // Save callbacks
  index = alloc_callback (streams_cb);

  if (index < 0)
    {
      caml_failwith ("Max probe number exceeded");
    }
  
  caml_register_global_root(&closures[index]);
  streams_funs.id = index;

  caml_release_runtime_system ();
        
  probe = qoe_probe_create (&args,
                            streams_funs,
                            &error);

  caml_stat_free (args.tag);
  caml_stat_free (args.arg1);
        
  caml_acquire_runtime_system ();
  //printf ("Context received at %p\n", context);
  // Save the context
  Probe_val(prb) = probe;
        
  if (probe == NULL)
    if (error)
      {
        caml_failwith (error);
        free (error);
      }
    else
      {
        caml_failwith ("Unknown error");
      }

  rval = caml_alloc_tuple (2);

  Store_field (rval, 0, prb);
  Store_field (rval, 1, Val_int (index));
  
  CAMLreturn(rval);
}

CAMLprim value
caml_qoe_probe_run (value tuple)
{
  CAMLparam1 (tuple);
  //CAMLlocal0 ();

  Probe * probe = Probe_val (Field (tuple, 0));

  if (probe == NULL)
    caml_failwith ("Invalid probe");
  //printf ("Context received at %p\n", back);

  caml_release_runtime_system ();

  qoe_probe_run (probe);

  caml_acquire_runtime_system ();
        
  CAMLreturn (Val_unit);
}

CAMLprim value
caml_qoe_probe_free (value tuple)
{
  CAMLparam1 (tuple);
  //CAMLlocal0 ();
  // TODO properly deallocate
  Probe * probe = Probe_val (Field (tuple, 0));  
  int32_t index = Int_val (Field (tuple, 1));
  
  if (probe == NULL)
    caml_failwith ("Invalid probe");

  caml_release_runtime_system ();

  qoe_probe_free (probe);

  caml_acquire_runtime_system ();

  Probe_val (probe) = NULL;

  caml_remove_global_root(&closures[index]);
  dealloc_callback (index);
  //streams_closure = NULL;
        
  CAMLreturn(Val_unit);
}

CAMLprim value
caml_qoe_probe_get_structure (value tuple)
{
  CAMLparam1 (tuple);
  CAMLlocal1 (res);
  // TODO properly deallocate
  Probe * probe = Probe_val (Field (tuple, 0));
  char  * streams;

  if (probe == NULL)
    caml_failwith ("Invalid probe");

  caml_release_runtime_system ();

  streams = qoe_probe_get_structure (probe);

  caml_acquire_runtime_system ();
  // TODO reduce allocations
  res = caml_copy_string(streams);
  free (streams);
        
  CAMLreturn(res);
}
