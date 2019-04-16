module Make
         (Id : Qoe_backend_types.Basic.STREAM_ID)
         (Uri_string : Qoe_backend_types.Basic.URI)
         (Useconds : Qoe_backend_types.Basic.USECONDS)
       : sig

  module Structure : module type of Qoe_backend_types.Structure.Make (Id) (Uri_string)

  module Wm : module type of Qoe_backend_types.Wm.Make (Id)

  module Qoe_errors : module type of Qoe_backend_types.Qoe_errors.Make (Id) (Useconds)

  module Qoe_status : module type of Qoe_backend_types.Qoe_status.Make (Id)
  
  type t

  type events = { streams : Structure.t list React.event
                ; graph : Structure.t list React.event
                ; wm : Wm.t React.event
                ; vdata : Qoe_errors.Video_data.t React.event
                ; adata : Qoe_errors.Audio_data.t React.event
                }
              
  val init_logger : unit -> unit
    
  val create : (string * string) array -> t * events

  val run : t -> unit Lwt.t

  val destroy : t -> unit

end
