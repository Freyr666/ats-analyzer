module Make
         (Id : Qoe_backend_types.Basic.STREAM_ID)
         (Time : Qoe_backend_types.Basic.USECONDS)
         (Time_span : Qoe_backend_types.Basic.USECONDS_SPAN)
       : sig

  module Qoe_errors : module type of Qoe_backend_types.Qoe_errors.Make(Id)(Time)(Time_span)

  val video_errors : Gstbuffer.t -> Id.t -> int -> int -> Qoe_errors.Video_data.t

  val audio_errors : Gstbuffer.t -> Id.t -> int -> int -> Qoe_errors.Audio_data.t
  
end
