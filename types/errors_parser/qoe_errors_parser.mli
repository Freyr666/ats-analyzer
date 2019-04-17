module Make
         (Id : Qoe_backend_types.Basic.STREAM_ID)
         (Time : Qoe_backend_types.Basic.USECONDS)
       : sig

  module Qoe_errors : module type of Qoe_backend_types.Qoe_errors.Make(Id)(Time)
  
  val get_video_errors : Gstbuffer.t -> Qoe_errors.Video_data.errors

  val get_audio_errors : Gstbuffer.t -> Qoe_errors.Audio_data.errors
  
end
