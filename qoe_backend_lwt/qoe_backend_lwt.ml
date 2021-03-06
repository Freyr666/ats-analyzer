type error = [ `Qoe_backend of string ]

module Make
         (Id : Qoe_backend_types.Basic.STREAM_ID)
         (Uri_string : Qoe_backend_types.Basic.URI)
         (Useconds : Qoe_backend_types.Basic.USECONDS)
         (Useconds_span : Qoe_backend_types.Basic.USECONDS_SPAN)
  = struct

  module Structure = Qoe_backend_types.Structure.Make (Id) (Uri_string)

  module Wm = Qoe_backend_types.Wm.Make (Id)

  module Settings = Qoe_backend_types.Settings.Make (Id)

  module Qoe_errors = Qoe_backend_types.Qoe_errors.Make (Id) (Useconds) (Useconds_span)

  module Qoe_status = Qoe_backend_types.Qoe_status.Make (Id)

  module Qoe_error_parser = Qoe_errors_parser.Make (Id) (Useconds) (Useconds_span)
  
  type t = Qoe_backend.t

  type events = { streams : Structure.t list React.event
                ; graph : Structure.t list React.event
                ; wm : Wm.t React.event
                ; vdata : Qoe_errors.Video_data.t React.event
                ; adata : Qoe_errors.Audio_data.t React.event
                ; status : Qoe_status.t React.event
                }
              
  let init_logger = Qoe_backend.init_logger

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

  let make_data_event ()  =
    let video, push_v = Lwt_react.E.create () in
    let audio, push_a = Lwt_react.E.create () in
    let data_vid = ref None in
    let data_aud = ref None in
    let notif_vid = Lwt_unix.make_notification
                      (fun () -> match !data_vid with
                                 | None -> ()
                                 | Some data -> push_v data)
    in
    let notif_aud = Lwt_unix.make_notification
                      (fun () -> match !data_aud with
                                 | None -> ()
                                 | Some data -> push_a data)
    in
    let cb typ id channel pid buf =
      match typ with
      | Qoe_backend.Video -> begin
         try let errors = Qoe_error_parser.video_errors buf (Id.of_string id) channel pid in 
             data_vid := Some errors;
             Lwt_unix.send_notification notif_vid
         with Failure _ -> () (* TODO log errors *)
            | _ -> ()
        end
      | Qoe_backend.Audio -> begin
          try let errors = Qoe_error_parser.audio_errors buf (Id.of_string id) channel pid in 
              data_aud := Some errors;
              Lwt_unix.send_notification notif_aud
          with Failure _ -> () (* TODO log errors *)
             | _ -> ()
        end
    in cb, video, audio

  let make_status_event ()  =
    let open Qoe_status in
    let e, push = Lwt_react.E.create () in
    let data = ref None in
    let notif = Lwt_unix.make_notification
                  (fun () -> match !data with
                             | None -> ()
                             | Some data -> push data)
    in
    let cb id channel pid playing =
      data := Some { stream = Id.of_string id
                   ; channel
                   ; pid
                   ; playing
                };
      Lwt_unix.send_notification notif
    in cb, e

  let of_json conv js =
    Yojson.Safe.from_string js
    |> conv
    |> function Ok v -> v | Error e -> failwith e

  let of_json_err conv js =
    Yojson.Safe.from_string js
    |> conv
    |> function Ok _ as v -> v
              | Error e -> Error (`Qoe_backend e)

  let to_json conv v =
    conv v
    |> Yojson.Safe.to_string
                         
  let get conv f x =
    Lwt_preemptive.detach (fun obj ->
        try f obj
            |> of_json_err conv
        with Failure e -> Error (`Qoe_backend e))
      x

  let set conv f data x =
    Lwt_preemptive.detach (fun obj ->
        try Ok (f obj (to_json conv data))
        with Failure e -> Error (`Qoe_backend e))
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
      let args' = Array.map to_string args in
      let streams_cb, streams = make_event (of_json Structure.many_of_yojson) in
      let graph_cb, graph = make_event (of_json Structure.many_of_yojson) in
      let wm_cb, wm = make_event (of_json Wm.of_yojson) in
      let data_cb, vdata, adata = make_data_event () in
      let status_cb, status = make_status_event () in
      let backend = Qoe_backend.create args'
                      ~streams:streams_cb
                      ~graph:graph_cb
                      ~wm:wm_cb
                      ~data:data_cb
                      ~status:status_cb
      and events = { streams
                   ; graph
                   ; wm
                   ; vdata
                   ; adata
                   ; status
                   }
      in Lwt.return_ok (backend, events)
    with Failure e -> Lwt.return_error (`Qoe_backend e)
       | _ -> Lwt.return_error (`Qoe_backend "bad params")

  let run backend =
    Lwt_preemptive.detach Qoe_backend.run backend

  let destroy backend =
    Qoe_backend.free backend

  module Stream_parser = struct

    let get_structure backend =
      get
        Structure.many_of_yojson
        Qoe_backend.stream_parser_get_structure
        backend

  end

  module Graph = struct

    let get_structure backend =
      get
        Structure.many_of_yojson
        Qoe_backend.graph_get_structure
        backend
      
    let apply_structure backend data =
      set
        Structure.many_to_yojson
        Qoe_backend.graph_apply_structure
        data
        backend

  end

  module Mosaic = struct

    let get_layout backend =
      get
        Wm.of_yojson
        Qoe_backend.wm_get_layout
        backend
      
    let apply_layout backend data =
      set
        Wm.to_yojson
        Qoe_backend.wm_apply_layout
        data
        backend

  end

  module Analysis_settings = struct

    let get_settings backend =
      get
        Settings.of_yojson
        Qoe_backend.graph_get_settings
        backend

    let apply_settings backend data =
      set
        Settings.to_yojson
        Qoe_backend.graph_apply_settings
        data
        backend

  end
               
end
