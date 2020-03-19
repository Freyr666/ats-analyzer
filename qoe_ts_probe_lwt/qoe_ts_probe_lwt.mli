type error = [ `Qoe_ts_probe of string ]

module Make (Id : Qoe_media_stream.Basic.STREAM_ID) : sig

  module Structure : module type of Qoe_media_stream.Structure.Make (Id)
  
  type t = Qoe_ts_probe.t

  val create : (Id.t * Uri.t) -> ((t * Structure.t React.event), [> error]) Lwt_result.t

  val run : t -> unit Lwt.t

  val destroy : t -> unit

  val get_structure : t -> (Structure.t, [> error]) Lwt_result.t

end
