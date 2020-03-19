open Qoe_media_stream.Basic

module Make (Id : STREAM_ID) = struct

  module Pid_map = Map.Make (struct
                       type t = Id.t * int * int
                       (* TODO proper compare *)
                       let compare : t -> t -> int = compare
                     end)
                 
  type setting =
    { peak_en  : bool
    ; peak     : float
    ; cont_en  : bool
    ; cont     : float
    ; duration : float
    } [@@deriving yojson,eq]

  type black =
    { black       : setting
    ; luma        : setting
    ; black_pixel : int
    } [@@deriving yojson,eq]

  type freeze =
    { freeze     : setting
    ; diff       : setting
    ; pixel_diff : int
    } [@@deriving yojson,eq]

  type blocky =
    { blocky  : setting
    } [@@deriving yojson,eq]

  type silence =
    { silence : setting
    } [@@deriving yojson,eq]

  type loudness =
    { loudness : setting
    } [@@deriving yojson,eq]

  type adv =
    { adv_diff : float
    ; adv_buf  : int
    } [@@deriving yojson,eq]

  type video =
    { loss     : float
    ; black    : black
    ; freeze   : freeze
    ; blocky   : blocky
    } [@@deriving yojson,eq]
    
  type audio =
    { loss     : float
    ; silence  : silence
    ; loudness : loudness
    ; adv      : adv
    } [@@deriving yojson,eq]
    
  type t =
    { default_video : video
    ; default_audio : audio
    ; video : video Pid_map.t
    ; audio : audio Pid_map.t
    } [@@deriving eq]

  let to_yojson v : Yojson.Safe.t =
    let video =
      Pid_map.to_seq v.video
      |> Seq.map (fun ((s,c,p),d) ->
             `List [Id.to_yojson s; `Int c; `Int p; video_to_yojson d])
      |> fun seq -> `List (List.of_seq seq)
    and audio =
      Pid_map.to_seq v.audio
      |> Seq.map (fun ((s,c,p),d) ->
             `List [Id.to_yojson s; `Int c; `Int p; audio_to_yojson d])
      |> fun seq -> `List (List.of_seq seq)
    in
    `Assoc [ "default_video", video_to_yojson v.default_video
           ; "default_audio", audio_to_yojson v.default_audio
           ; "video", video
           ; "audio", audio
      ]

  let of_yojson (js : Yojson.Safe.t) : (t,string) result =
    let (>>=) r f = match r with
      | Ok x -> f x
      | Error _ as e -> e
    in
    let vlist_to_map l =
      try
        Ok (List.fold_left (fun acc e ->
                match e with
                | `List [s; `Int c; `Int p; v] ->
                   begin match (* TODO redesign after 4.08 *)
                     Id.of_yojson s >>= fun id ->
                     video_of_yojson v >>= fun v ->
                     Ok (id, v)
                   with 
                   | Error e -> failwith e
                   | Ok (id,v) -> Pid_map.add (id,c,p) v acc
                   end
                | _ -> failwith "Settings.of_yojson: bad video settings")
              Pid_map.empty l)
      with Failure e -> Error e
    in
    let alist_to_map l =
      try
        Ok (List.fold_left (fun acc e ->
                match e with
                | `List [s; `Int c; `Int p; v] ->
                   begin match
                     Id.of_yojson s >>= fun id ->
                     audio_of_yojson v >>= fun v ->
                     Ok (id, v)
                   with
                   | Error e -> failwith e
                   | Ok (id,v) -> Pid_map.add (id,c,p) v acc
                   end
                | _ -> failwith "Settings.of_yojson: bad audio settings")
              Pid_map.empty l)
      with Failure e -> Error e
    in
    match js with
    | `Assoc [ "default_video", dv
             ; "default_audio", da
             ; "video", `List vl
             ; "audio", `List al
      ] ->
       video_of_yojson dv >>= fun default_video ->
       audio_of_yojson da >>= fun default_audio ->
       vlist_to_map vl >>= fun video ->
       alist_to_map al >>= fun audio ->
       Ok { default_video; default_audio; video; audio }
    | _ -> Error "Settings.of_yojson"

  let black_default = { black = { peak_en = true
                                ; peak    = 100.0
                                ; cont_en = false
                                ; cont    = 90.0
                                ; duration = 10.
                                }
                      ; luma  = { peak_en = false
                                ; peak    = 20.0
                                ; cont_en = true
                                ; cont    = 17.0
                                ; duration = 10.
                                }
                      ; black_pixel = 16
                      }

  let freeze_default = { freeze  = { peak_en = true
                                   ; peak    = 99.0
                                   ; cont_en = false
                                   ; cont    = 80.0
                                   ; duration = 10.
                                   }
                       ; diff    = { peak_en = false
                                   ; peak    = 0.1
                                   ; cont_en = true
                                   ; cont    = 0.02
                                   ; duration = 10.
                                   }
                       ; pixel_diff  = 2
                       }

  let blocky_default = { blocky = { peak_en = true
                                  ; peak    = 7.0
                                  ; cont_en = false
                                  ; cont    = 4.0
                                  ; duration = 10.
                                  }
                       }

  let silence_default = { silence = { peak_en = false
                                    ; peak    = (-35.0)
                                    ; cont_en = true
                                    ; cont    = (-33.0)
                                    ; duration = 3.
                                    }
                        }

  let loudness_default = { loudness = { peak_en = true
                                      ; peak    = (-15.0)
                                      ; cont_en = true
                                      ; cont    = (-22.0)
                                      ; duration = 4.
                                      }
                         }

  let adv_default = { adv_diff = 1.5
                    ; adv_buf = 3200
                    }
                  
  let default = { default_video = { loss = 2.0
                                  ; black = black_default
                                  ; freeze = freeze_default
                                  ; blocky = blocky_default
                                  }
                ; default_audio = { loss = 3.0
                                  ; silence = silence_default
                                  ; loudness = loudness_default
                                  ; adv = adv_default
                                  }
                ; video = Pid_map.empty
                ; audio = Pid_map.empty
                }

  let to_string s = Yojson.Safe.to_string (to_yojson s)
  let of_string s =
    match of_yojson (Yojson.Safe.from_string s) with
    | Ok v -> v
    | Error e -> failwith e

  let combine ~set x = ignore set; `Kept x

end
