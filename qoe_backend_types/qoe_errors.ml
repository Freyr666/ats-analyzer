open Qoe_media_stream.Basic

type time = Ptime.t

type time_span = Ptime.Span.t

let time_span_to_yojson (v : time_span) : Yojson.Safe.t =
  let d, ps = Ptime.Span.to_d_ps v in
  `List [`Int d; `Intlit (Int64.to_string ps)]

let time_span_of_yojson (j : Yojson.Safe.t) : (time_span, string) result =
  let to_err j =
    Printf.sprintf "time_span_of_yojson: bad json value (%s)"
    @@ Yojson.Safe.to_string j in
  match j with
  | `List [`Int d; `Intlit ps] ->
     begin match Int64.of_string_opt ps with
     | None -> Error "time_span_of_yojson: error"
     | Some ps ->
        Ok (Ptime.to_span @@ Ptime.v (d, ps))
     end
  | _ -> Error (to_err j)

let time_to_yojson (v : time) : Yojson.Safe.t =
  time_span_to_yojson @@ Ptime.to_span v

let time_of_yojson (j : Yojson.Safe.t) : (time, string) result =
  match time_span_of_yojson j with
  | Error _ as e -> e
  | Ok v ->
     match Ptime.of_span v with
     | Some v -> Ok v
     | None -> Error "time_of_yojson: bad time span"
   
module Make (Id : STREAM_ID) = struct

  type point =
    { time : time
    ; data : float
    } [@@deriving yojson]

  type flag =
    { value : bool
    ; time  : time
    ; span  : time_span
    } [@@deriving yojson]
  
  type error =
    { cont_flag : flag
    ; peak_flag : flag
    } [@@deriving yojson]

  module Video_data = struct
    type errors =
      { black  : error
      ; luma   : error
      ; freeze : error
      ; diff   : error
      ; blocky : error
      } [@@deriving yojson]

    type data =
      { black  : point array
      ; luma   : point array
      ; freeze : point array
      ; diff   : point array
      ; blocky : point array
      } [@@deriving yojson]
      
    type t =
      { stream     : Id.t
      ; channel    : int
      ; pid        : int
      ; errors     : errors
      ; data       : data
      } [@@deriving yojson]
      
  end

  module Audio_data = struct
    
    type errors =
      { silence_shortt  : error
      ; silence_moment  : error
      ; loudness_shortt : error
      ; loudness_moment : error
      } [@@deriving yojson]

    type data =
      { shortt : point array
      ; moment : point array
      } [@@deriving yojson]
      
    type t =
      { stream     : Id.t
      ; channel    : int
      ; pid        : int
      ; errors     : errors
      ; data       : data
      } [@@deriving yojson]
  end

  type labels =
    [ `Black
    | `Luma
    | `Freeze
    | `Diff
    | `Blocky
    | `Silence_shortt
    | `Silence_moment
    | `Loudness_shortt
    | `Loudness_moment
    ] [@@deriving yojson, eq]
(*
  let video_data_to_list Video_data.{ stream; channel; pid; errors = { black; luma; freeze; diff; blocky } } =
    [ stream, channel, pid, 0, black
    ; stream, channel, pid, 1, luma
    ; stream, channel, pid, 2, freeze
    ; stream, channel, pid, 3, diff
    ; stream, channel, pid, 4, blocky
    ]

  let audio_data_to_list Audio_data.{ stream; channel; pid; errors = { silence_shortt; silence_moment; loudness_shortt; loudness_moment } } =
    [ stream, channel, pid, 5, silence_shortt
    ; stream, channel, pid, 6, loudness_shortt
    ; stream, channel, pid, 7, silence_moment
    ; stream, channel, pid, 8, loudness_moment
    ]
 *)
  let labels_of_int = function
    | 0 -> `Black
    | 1 -> `Luma
    | 2 -> `Freeze
    | 3 -> `Diff
    | 4 -> `Blocky
    | 5 -> `Silence_shortt
    | 6 -> `Loudness_shortt
    | 7 -> `Silence_moment
    | 8 -> `Loudness_moment
    | _ -> failwith "Qoe_errors.labels_of_int: wrong int"

  let labels_to_int = function
    | `Black -> 0
    | `Luma -> 1
    | `Freeze -> 2
    | `Diff -> 3
    | `Blocky -> 4
    | `Silence_shortt -> 5
    | `Loudness_shortt -> 6
    | `Silence_moment -> 7
    | `Loudness_moment -> 8

end
