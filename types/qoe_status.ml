open Basic

module Make (Id : STREAM_ID) = struct

  type t =
    { stream  : Id.t
    ; channel : int
    ; pid     : int
    ; playing : bool
    } [@@deriving yojson, ord]

  type status_list = t list [@@deriving yojson]

end
