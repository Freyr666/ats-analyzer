
type t

type id = string

type url = string
   
external create : (id * url) -> streams:(string -> unit) -> t =
  "caml_qoe_probe_create"

external run : t -> unit = "caml_qoe_probe_run"

external free : t -> unit = "caml_qoe_probe_free"

external get_structure : t -> string =
  "caml_qoe_probe_get_structure"
