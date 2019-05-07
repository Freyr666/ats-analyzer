open Lwt.Infix

let get_exn = function Some v -> v | None -> failwith "get exn None"

let to_result = function Some v -> Ok v | None -> Error "to res None"

module I64 = struct
  include Int64
  let ( * ) = mul
  let ( / ) = div
end

module Int = struct
  let (mod) = (mod)
end

module ID = struct
  (* TODO remove in 4.08 *)
  let get_exn = function Some v -> v | None -> failwith "None"

  let of_opt = function Some v -> Ok v | None -> Error "None"

  include Uuidm

  type api_fmt = t

  let typ = "uuid"

  let to_string (x : t) = to_string x
  let of_string_opt (s : string) = of_string s
  let of_string (s : string) = get_exn @@ of_string_opt s

  let to_yojson (x : t) : Yojson.Safe.json =
    `String (Uuidm.to_string x)
  let of_yojson : Yojson.Safe.json -> (t, string) result = function
    | `String s -> of_opt @@ Uuidm.of_string s
    | _ -> Error "uuid_of_yojson: not a string"

  let make (s : string) =
    v5 ns_url s

end

module Period = struct
  include Ptime

  let ps_in_s = 1000_000_000_000L

  let to_yojson (v:t) : Yojson.Safe.json =
    let d, ps = Ptime.Span.to_d_ps @@ Ptime.to_span v in
    `List [ `Int d;`Intlit (Int64.to_string ps) ]

  let of_yojson (j:Yojson.Safe.json) : (t,string) result =
    let to_err j = Printf.sprintf "span_of_yojson: bad json value (%s)" @@ Yojson.Safe.to_string j in
    match j with
    | `List [ `Int d; `Intlit ps] -> (match Int64.of_string_opt ps with
                                      | Some ps -> to_result (match Ptime.Span.of_d_ps (d,ps)
                                                              with None -> None
                                                                 | Some s -> Ptime.of_span s)
                                      | None    -> Error (to_err j))
    | _ -> Error (to_err j)

  module Conv (M : sig
               val of_int : int -> int * int64
               val to_int : int * int64 -> int
             end) = struct
    type t = Ptime.Span.t
           
    let of_int x = get_exn @@ Ptime.Span.of_d_ps (M.of_int x)
    let to_int x = M.to_int @@ Ptime.Span.to_d_ps x
    let of_string s = of_int @@ int_of_string s
    let to_string x = string_of_int @@ to_int x
    let of_yojson x = match x with `Int x -> Ok(of_int x)
                                 | _ -> Error "of_yojson"
                                 | exception _ -> Error "of_yojson"
    let to_yojson x = `Int (to_int x)
  end

  module Conv64 (M : sig val second : int64 end) = struct
    let () =
      if Int64.compare M.second ps_in_s > 0
      then failwith "Time.Span.Conv64: second precision is more than 1ps"

    type t = Ptime.t

    let of_int64 (x : int64) : t =
      Ptime.of_float_s ((Int64.to_float x) /. (Int64.to_float M.second))
      |> get_exn
    (* let d  = Int64.(to_int (x / (24L * 60L * 60L * M.second))) in
     * let ps = Int64.((x mod (24L * 60L * 60L)) * (ps_in_s / M.second)) in
     * Option.get_exn
     * @@ Option.flat_map Ptime.of_span (Ptime.Span.of_d_ps (d, ps)) *)

    let to_int64 (x : t) : int64 =
      Ptime.to_float_s x
      |> ( *. ) (Int64.to_float M.second)
      |> Int64.of_float
    (* let d, ps = Ptime.Span.to_d_ps @@ Ptime.to_span x in
     * let d = Int64.((of_int d) * (24L * 60L * 60L * M.second)) in
     * let ps = Int64.((ps * M.second) / ps_in_s) in
     * Int64.(d + ps) *)

    let of_string (s : string) : t =
      of_int64 @@ Int64.of_string s
    let to_string (x : t) : string =
      Int64.to_string @@ to_int64 x

    let of_yojson (x : Yojson.Safe.json) : (t, string) result = match x with
      | `Intlit x -> Ok(of_string x)
      | `Int x -> Ok(of_int64 @@ Int64.of_int x)
      | _ -> Error "of_yojson"
      | exception _ -> Error "of_yojson"

    let to_yojson (x : t) : Yojson.Safe.json =
      `Intlit (to_string x)

  end

  module Hours = Conv(struct
                     let of_int x = (x / 24, I64.(of_int Int.(x mod 24) * 3600L * ps_in_s))
                     let to_int (d,ps) = (d * 24) + I64.(to_int (ps / (3600L * ps_in_s)))
                   end)

  module Seconds = Conv(struct
                       let of_int x = Ptime.Span.to_d_ps @@ Ptime.Span.of_int_s x
                       let to_int x = get_exn @@ Ptime.Span.to_int_s @@ Ptime.Span.v x
                     end)

  module Seconds64 = Conv64(struct let second = 1L end)
  module Useconds = Conv64(struct let second = 1000_000L end) 
                  
end

module Qoe = Qoe_backend_lwt.Make
               (ID)
               (struct
                 include Uri
                 let to_yojson u = `String (to_string u)
                 let of_yojson = function (`String u : Yojson.Safe.json) -> Ok (of_string u)
                                        | _ -> Error "bad uri"
               end)
               (Period.Useconds)

let main () =
  let (>>=?) = Lwt_result.bind in
  Qoe.init_logger ();
  Qoe.create [| (ID.to_string @@ ID.make "test"),
                "udp://224.1.2.2:1234" |]
  >>=? fun (back, events) ->
  Gc.full_major ();
  Lwt_react.E.keep
  @@ Lwt_react.E.map_p (fun (s : Qoe.Structure.t list) ->
         Lwt_io.printf "Got graph data: %s\n" (Yojson.Safe.pretty_to_string @@ Qoe.Structure.many_to_yojson  s))
       events.graph;
  Lwt_react.E.keep
  @@ Lwt_react.E.map_p (fun (s : Qoe.Qoe_status.t) ->
         Lwt_io.printf "Got status: %d %d %b\n" s.channel s.pid s.playing)
       events.status;
  Lwt_react.E.keep
  @@ Lwt_react.E.map_p (fun (s : Qoe.Qoe_errors.Video_data.t) ->
         Lwt_io.printf "Got vdata: %d %d freeze frame: %b %b %f %f %f\n" s.channel s.pid s.errors.freeze.peak_flag s.errors.freeze.cont_flag s.errors.freeze.params.max s.errors.freeze.params.avg s.errors.freeze.params.avg)
       events.vdata;
  Gc.finalise (fun _ -> print_endline "Backend was collected") back;
  let t = Qoe.run back in
  Gc.full_major ();
  Lwt_unix.sleep 10.0
  >>= fun () ->
  Qoe.Stream_parser.get_structure back
  >>=? fun s ->
  Lwt_io.printf "Streams: %s\n"
    (Yojson.Safe.pretty_to_string @@ Qoe.Structure.many_to_yojson s)
  >>= fun () ->
  Qoe.Graph.apply_structure back s
  >>=? fun () ->
  Lwt_unix.sleep 10.0
  >>= fun () ->
  Qoe.Graph.get_structure back
  >>=? fun s ->
  Lwt_io.printf "Applied: %s\n"
    (Yojson.Safe.pretty_to_string @@ Qoe.Structure.many_to_yojson s)
  >>= fun () ->
  Lwt_unix.sleep 20.0
  >>= fun () ->
  Gc.full_major ();
  Lwt_unix.sleep 20.0
  >>= fun () ->
  Lwt_io.printf "destroying back\n"
  >>= fun () ->
  Qoe.destroy back;
  t >>= fun () ->
  Lwt.return_ok ()

let () =
  match Lwt_main.run (main ()) with
  | Ok () -> ()
  | Error (`Qoe_backend e) -> Printf.printf "Exited with error: %s\n" e
