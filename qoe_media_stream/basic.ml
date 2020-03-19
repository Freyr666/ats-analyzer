module type STREAM_ID = sig
  type t (* = Uuidm.t *)
  val compare : t -> t -> int
  val equal : t -> t -> bool
  val of_yojson : Yojson.Safe.t -> (t, string) result
  val to_yojson : t -> Yojson.Safe.t
  val of_string : string -> t
  val to_string : t -> string
end
