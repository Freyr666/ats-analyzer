type t

external init_logger : unit -> unit = "caml_qoe_backend_init_logger"

external create : unit -> t = "caml_qoe_backend_create"

(* Blocks *)
external run : t -> unit = "caml_qoe_backend_run"

external quit : t -> unit = "caml_qoe_backend_quit"

external free : t -> unit = "caml_qoe_backend_free"

external get_streams : t -> string = "caml_qoe_backend_get_streams"
                             
