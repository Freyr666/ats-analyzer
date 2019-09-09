type error = [ `Qoe_backend of string ]

module Make
         (Id : Qoe_backend_types.Basic.STREAM_ID)
       : sig

  module Structure : module type of Qoe_backend_types.Structure.Make (Id)

  module Wm : module type of Qoe_backend_types.Wm.Make (Id)

  module Settings : module type of Qoe_backend_types.Settings.Make (Id)

  module Qoe_errors : module type of Qoe_backend_types.Qoe_errors.Make (Id)

  module Qoe_status : module type of Qoe_backend_types.Qoe_status.Make (Id)
  
  type t = Qoe_backend.t

  type events = { streams : Structure.t list React.event
                ; graph : Structure.t list React.event
                ; wm : Wm.t React.event
                ; vdata : Qoe_errors.Video_data.t React.event
                ; adata : Qoe_errors.Audio_data.t React.event
                ; status : Qoe_status.t React.event
                }
              
  val init_logger : unit -> unit
    
  val create : (Id.t * Uri.t) array -> ((t * events), [> error]) Lwt_result.t

  val run : t -> unit Lwt.t

  val destroy : t -> unit

  module Stream_parser : sig

    val get_structure : t -> (Structure.t list, [> error]) Lwt_result.t

  end

  module Graph : sig

    val get_structure : t -> (Structure.t list, [> error]) Lwt_result.t

    val apply_structure : t -> Structure.t list -> (unit, [> error]) Lwt_result.t

  end

  module Mosaic : sig

    val get_layout : t -> (Wm.t, [> error]) Lwt_result.t

    val apply_layout : t -> Wm.t -> (unit, [> error]) Lwt_result.t

  end

  module Analysis_settings : sig

    val get_settings : t -> (Settings.t, [> error]) Lwt_result.t

    val apply_settings : t -> Settings.t -> (unit, [> error]) Lwt_result.t

  end

end
