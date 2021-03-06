type t

type buf = (char, Bigarray.int8_unsigned_elt, Bigarray.c_layout) Bigarray.Array1.t

(**
   Unsafe! Do not even try to keep the buf reference in
   the callback result, the bigarray would be invalidated
   right after the callback would return its value
 *)
val process_unsafe : t -> (buf -> 'a) -> 'a
