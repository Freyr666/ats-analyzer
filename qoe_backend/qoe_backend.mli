type t

type typ = Video | Audio
                 
val init_logger : unit -> unit

(* TODO { type (SDI, external stream, internal UUID stream);
 *        tag:  stream's name or UUID;
 *        arg1: stream's address or V4L2 video source
 *        arg2: alsa audio source
 *      }
 *)
  
val create : (string * string) array
             -> streams:(string -> unit)
             -> graph:(string -> unit)
             -> wm:(string -> unit)
             -> data:(typ -> string -> int -> int -> Gstbuffer.t -> unit)
             -> status:(string -> int -> int -> bool -> unit)
             -> t

(* Blocks *)
val run : t -> unit

val free : t -> unit

val stream_parser_get_structure : t -> string
                             
val graph_get_structure : t -> string

val graph_apply_structure : t -> string -> unit

val wm_get_layout : t -> string

val wm_apply_layout : t -> string -> unit
