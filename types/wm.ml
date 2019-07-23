(* NOTE: just an assumption *)
(*
type widget_type = Video
                 | Audio
                 | Subtitles
                 | Teletext
                 | Qos          (* qos errors/events *)
                 | Qoe          (* qoe errors/events *)
                 | Clock        (* digital or analog clock *)
                 | Text         (* static text *)
                 | Image        (* image (from file or from url, maybe) *)
                 | Source       (* text with channel(stream,input,etc) name*)
                 | Eit          (* text with EIT info *)
                 | Service_info (* text with service desrciption (resolution,codec,etc) *)
                 | Icons_bar    (* status bar with availability indication of eit, scte35, teletext etc  *)
[@@deriving yojson]
 *)

open Basic

module Make (Id : STREAM_ID) = struct

  type widget_type = Video | Audio [@@deriving eq]

  let widget_type_equal typ_1 typ_2 =
    match typ_1, typ_2 with
    | Video, Video | Audio, Audio -> true
    | _ -> false

  let widget_type_of_yojson = function
    | `String "Video" -> Ok(Video)
    | `String "Audio" -> Ok(Audio)
    | _ -> Error "widget_type_of_yojson"
  let widget_type_to_yojson = function
    | Video -> `String "Video"
    | Audio -> `String "Audio"

  type domain = Nihil : domain
              | Chan  : { stream  : Id.t
                        ; channel : int }
                        -> domain
                             [@@deriving eq]
  let domain_of_yojson = function
    | `String "Nihil" -> Ok(Nihil)
    | `Assoc ["Chan",
              `Assoc ["stream", `String id;
                      "channel", `Int channel]] ->
       Ok(Chan { stream = Id.of_string id; channel })
    | `Assoc ["Chan",
              `Assoc ["stream", `String id;
                      "channel", `Intlit channel]] ->
       Ok(Chan { stream = Id.of_string id; channel = int_of_string channel })
    | _ -> Error "domain_of_yojson: bad json"
  let domain_to_yojson = function
    | Nihil -> `String "Nihil"
    | Chan { stream; channel } ->
       `Assoc ["Chan", `Assoc ["stream", `String (Id.to_string stream);
                               "channel", `Int channel]]
      
  type background = (* NOTE incomplete *)
    { color : int } [@@deriving yojson]

  type position =
    { x : float
    ; y : float
    ; w : float
    ; h : float
    } [@@deriving yojson, eq]

  type widget =
    { type_       : widget_type [@key "type"]
    ; domain      : domain
    ; pid         : int option
    ; position    : position option
    ; layer       : int
    ; aspect      : ((int * int) option [@default None])
    ; description : string
    } [@@deriving yojson, eq]

  type container =
    { position : position
    ; widgets  : (string * widget) list
    } [@@deriving yojson, eq]

  type t =
    { resolution : int * int
    ; widgets    : (string * widget) list
    ; layout     : (string * container) list
    } [@@deriving yojson, eq]

  let aspect_to_string = function
    | None -> "none"
    | Some (x,y) -> Printf.sprintf "%dx%d" x y
                  
  let to_string w = Yojson.Safe.to_string (to_yojson w)

  let of_string s =
    match of_yojson (Yojson.Safe.from_string s) with
    | Ok v -> v
    | Error e -> failwith e

end
