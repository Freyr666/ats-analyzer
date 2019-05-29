type t

type typ = Video | Audio
   
external init_logger : unit -> unit = "caml_qoe_backend_init_logger"

(* TODO { type (SDI, external stream, internal UUID stream);
 *        tag:  stream's name or UUID;
 *        arg1: stream's address or V4L2 video source
 *        arg2: alsa audio source
 *      }
 *)
external create : (string * string) array
                  -> streams:(string -> unit)
                  -> graph:(string -> unit)
                  -> wm:(string -> unit)
                  -> data:(typ -> string -> int -> int -> Gstbuffer.t -> unit)
                  -> status:(string -> int -> int -> bool -> unit)
                  -> t =
  "caml_qoe_backend_create_bytecode"
    "caml_qoe_backend_create_native"

(* Blocks *)
external run : t -> unit = "caml_qoe_backend_run"

external free : t -> unit = "caml_qoe_backend_free"

external stream_parser_get_structure : t -> string =
  "caml_qoe_backend_stream_parser_get_structure"
                             
external graph_get_structure : t -> string =
  "caml_qoe_backend_graph_get_structure"

external graph_apply_structure : t -> string -> unit =
  "caml_qoe_backend_graph_apply_structure"

external graph_get_settings : t -> string =
  "caml_qoe_backend_graph_get_settings"

external graph_apply_settings : t -> string -> unit =
  "caml_qoe_backend_graph_apply_settings"

external wm_get_layout : t -> string =
  "caml_qoe_backend_wm_get_layout"

external wm_apply_layout : t -> string -> unit =
  "caml_qoe_backend_wm_apply_layout"
