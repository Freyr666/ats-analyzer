type buf = (char, Bigarray.int8_unsigned_elt, Bigarray.c_layout) Bigarray.Array1.t

module Make
         (Id : Qoe_backend_types.Basic.STREAM_ID)
         (Time : Qoe_backend_types.Basic.USECONDS)
       : sig

  module Qoe_errors : module type of Qoe_backend_types.Qoe_errors.Make(Id)(Time)
  
  val get_video_errors : buf -> Qoe_errors.Video_data.errors

  val get_audio_errors : buf -> Qoe_errors.Audio_data.errors
  
end
