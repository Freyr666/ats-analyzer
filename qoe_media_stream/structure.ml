open Basic

type uri = Uri.t

let equal_uri = Uri.equal

let uri_to_yojson x = `String (Uri.to_string x)

let uri_of_yojson = function
  | `String s -> begin
      try Ok (Uri.of_string s)
      with _ -> Error "uri_of_yojson"
    end
  | _ -> Error "uri_of_yojson"
   
module Make (Id : STREAM_ID) = struct
  
  type video_pid =
    { codec        : string
    ; resolution   : (int * int)
    ; aspect_ratio : (int * int)
    ; interlaced   : string
    ; frame_rate   : float
    } [@@deriving yojson,eq]

  type audio_pid =
    { codec       : string
    ; bitrate     : string
    ; channels    : int
    ; sample_rate : int
    } [@@deriving yojson,eq]

  type pid_content = Video of video_pid
                   | Audio of audio_pid
                   | Empty [@@deriving eq]
  let pid_content_to_yojson = function
    | Empty   -> `String "Empty"
    | Video v -> `Assoc [("Video", (video_pid_to_yojson v))]
    | Audio a -> `Assoc [("Audio", (audio_pid_to_yojson a))]
  let pid_content_of_yojson = function
    | `String "Empty" -> Ok(Empty)
    | `Assoc [("Video", v)] ->
       (match video_pid_of_yojson v with
        | Ok v -> Ok(Video v)
        | _    -> Error("failure in video_pid deserialize"))
    | `Assoc [("Audio", a)] ->
       (match audio_pid_of_yojson a with
        | Ok a -> Ok(Audio a)
        | _    -> Error("failure in audio_pid deserialize"))
    | _ -> Error("failure in pid_content deserialize")

  type pid =
    { pid              : int
    ; content          : pid_content
    ; stream_type      : int
    ; stream_type_name : string
    } [@@deriving yojson,eq]

  type channel =
    { number        : int
    ; service_name  : string
    ; provider_name : string
    ; pids          : pid list
    } [@@deriving yojson,eq]

  type t =
    { id       : Id.t
    ; uri      : uri
    ; channels : channel list
    } [@@deriving yojson,eq]

(*
  type many = t list [@@deriving yojson,eq]
 *)
end
