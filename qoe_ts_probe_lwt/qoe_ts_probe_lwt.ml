type error = [ `Qoe_ts_probe of string ]

module Make (Id :  Qoe_media_stream.Basic.STREAM_ID) = struct

  module Structure = Qoe_media_stream.Structure.Make (Id)

  type t = Qoe_ts_probe.t

  let make_event of_string =
    let e, push = Lwt_react.E.create () in
    let data = ref None in
    let notif = Lwt_unix.make_notification
                  (fun () -> match !data with
                             | None -> ()
                             | Some data -> push data)
    in
    let cb s =
      try data := Some (of_string s);
          Lwt_unix.send_notification notif
      with _ -> ()
    in
    cb, e

  let of_json conv js =
    Yojson.Safe.from_string js
    |> conv
    |> function Ok v -> v | Error e -> failwith e

  let of_json_err conv js =
    Yojson.Safe.from_string js
    |> conv
    |> function Ok _ as v -> v
              | Error e -> Error (`Qoe_ts_probe e)

  let get conv f x =
    Lwt_preemptive.detach (fun obj ->
        try f obj
            |> of_json_err conv
        with Failure e -> Error (`Qoe_ts_probe e))
      x
    
  (* TODO change args type to Id * URI *)
  let create args =
    let to_string (id, uri) =
      let id' = Id.to_string id
      and uri' =
        begin match Uri.scheme uri with
        | Some "udp" -> ()
        | _ -> failwith "uri should have scheme `udp`"
        end;
        begin match Uri.port uri with
        | None -> failwith "uri should have port"
        | _ -> ()
        end;
        Uri.to_string uri
      in
      id', uri'
    in
    try
      let args' = to_string args in
      let streams_cb, streams = make_event (of_json Structure.of_yojson) in
      let probe = Qoe_ts_probe.create args' ~streams:streams_cb
      in Lwt.return_ok (probe, streams)
    with Failure e -> Lwt.return_error (`Qoe_ts_probe e)
       | _ -> Lwt.return_error (`Qoe_ts_probe "bad params")

  let run probe =
    Lwt_preemptive.detach Qoe_ts_probe.run probe

  let destroy probe =
    Qoe_ts_probe.free probe

  let get_structure probe =
    get
      Structure.of_yojson
      Qoe_ts_probe.get_structure
      probe

end
