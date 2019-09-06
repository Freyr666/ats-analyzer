type error = [ `Qoe_ts_probe of string ]

module Make
         (Id : Qoe_backend_types.Basic.STREAM_ID)
         (Uri_string : Qoe_backend_types.Basic.URI)
         (Useconds : Qoe_backend_types.Basic.USECONDS)
         (Useconds_span : Qoe_backend_types.Basic.USECONDS_SPAN)
       : sig

  module Structure : module type of Qoe_backend_types.Structure.Make (Id) (Uri_string)
  
  type t = Qoe_ts_probe.t

  val create : (Id.t * Uri_string.t) -> ((t * Structure.t React.event), [> error]) Lwt_result.t

  val run : t -> unit Lwt.t

  val destroy : t -> unit

  val get_structure : t -> (Structure.t, [> error]) Lwt_result.t

end
