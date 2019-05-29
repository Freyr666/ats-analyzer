module type STREAM_ID = sig
  type t = Uuidm.t
  val compare : t -> t -> int
  val equal : t -> t -> bool
  val of_yojson : Yojson.Safe.json -> (t, string) result
  val to_yojson : t -> Yojson.Safe.json
  val of_string : string -> t
  val to_string : t -> string
end

module type URI = sig
  type t = Uri.t
  val equal : t -> t -> bool
  val of_yojson : Yojson.Safe.json -> (t, string) result
  val to_yojson : t -> Yojson.Safe.json
end
  
module type USECONDS = sig
  type t = Ptime.t
  val of_int64 : int64 -> t
  val of_yojson : Yojson.Safe.json -> (t, string) result
  val to_yojson : t -> Yojson.Safe.json
end

                       
