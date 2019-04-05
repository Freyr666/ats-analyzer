type t

external init_logger : unit -> unit = "caml_qoe_backend_init_logger"

external create : unit -> t = "caml_qoe_backend_create"
