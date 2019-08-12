open Qoe_backend_types.Basic
         
module Make (Id : STREAM_ID) = struct

  module Qoe_errors = Qoe_backend_types.Qoe_errors.Make (Id)

  external get_video_errors
           : Gstbuffer.t -> Qoe_errors.Video_data.errors * Qoe_errors.Video_data.data
    = "caml_video_errors_of_ba"

  external get_audio_errors
           : Gstbuffer.t -> Qoe_errors.Audio_data.errors * Qoe_errors.Audio_data.data
    = "caml_audio_errors_of_ba"

(*
  type parsed = (int * int * (float * float * float) * int64 * bool * bool)
                    
  type video_errors = { black  : parsed
                      ; luma   : parsed
                      ; freeze : parsed
                      ; diff   : parsed
                      ; blocky : parsed
                      }

  type audio_errors = { silence_shortt : parsed
                      ; loudness_shortt : parsed
                      ; silence_moment : parsed
                      ; loudness_moment : parsed
                      }

  let parsed_to_error (counter, size, params, ts, peak_flag, cont_flag) =
    let open Qoe_errors in
    let params_conv (min, max, avg) =
      { min; max; avg }
    in
    { counter
    ; size
    ; params = params_conv params
    ; timestamp = Time.of_int64 ts
    ; peak_flag
    ; cont_flag
    }
         
  external video_errors_of_ba : Gstbuffer.t -> video_errors = "caml_video_errors_of_ba"

  let get_video_errors ba =
    let open Qoe_errors.Video_data in
    let ve = video_errors_of_ba ba in
    { black = parsed_to_error ve.black
    ; luma = parsed_to_error ve.luma
    ; freeze = parsed_to_error ve.freeze
    ; diff = parsed_to_error ve.diff
    ; blocky = parsed_to_error ve.blocky
    }

  external audio_errors_of_ba : Gstbuffer.t -> audio_errors = "caml_audio_errors_of_ba"

  let get_audio_errors ba =
    let open Qoe_errors.Audio_data in
    let ae = audio_errors_of_ba ba in
    { silence_shortt = parsed_to_error ae.silence_shortt
    ; silence_moment = parsed_to_error ae.silence_moment
    ; loudness_shortt = parsed_to_error ae.loudness_shortt
    ; loudness_moment = parsed_to_error ae.loudness_moment
    }
 *)
    
end
