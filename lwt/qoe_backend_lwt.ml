module Make
         (Id : Qoe_backend_types.Basic.STREAM_ID)
         (Uri_string : Qoe_backend_types.Basic.URI)
         (Useconds : Qoe_backend_types.Basic.USECONDS)
  = struct

  module Structure = Qoe_backend_types.Structure.Make (Id) (Uri_string)

  module Wm = Qoe_backend_types.Wm.Make (Id)

  module Qoe_errors = Qoe_backend_types.Qoe_errors.Make (Id) (Useconds)

  module Qoe_status = Qoe_backend_types.Qoe_status.Make (Id)

  module Qoe_error_parser = Qoe_errors_parser.Make (Id) (Useconds)
  
  type t = Qoe_backend.t

  type events = { streams : Structure.t list React.event
                ; graph : Structure.t list React.event
                ; wm : Wm.t React.event
                ; vdata : Qoe_errors.Video_data.t React.event
                ; adata : Qoe_errors.Audio_data.t React.event
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

  let make_video_event ()  =
    let open Qoe_backend in
    let open Qoe_errors.Video_data in
    let e, push = Lwt_react.E.create () in
    let data = ref None in
    let notif = Lwt_unix.make_notification
                  (fun () -> match !data with
                             | None -> ()
                             | Some data -> push data)
    in
    let cb id channel pid buf =
      try let errors = Gstbuffer.process_unsafe buf Qoe_error_parser.get_video_errors in 
          data := Some { stream = Id.of_string id
                       ; channel
                       ; pid
                       ; errors
                    };
          Lwt_unix.send_notification notif
      with _ -> ()
    in cb, e

  let make_audio_event ()  =
    let open Qoe_backend in
    let open Qoe_errors.Audio_data in
    let e, push = Lwt_react.E.create () in
    let data = ref None in
    let notif = Lwt_unix.make_notification
                  (fun () -> match !data with
                             | None -> ()
                             | Some data -> push data)
    in
    let cb id channel pid buf =
      try let errors = Gstbuffer.process_unsafe buf Qoe_error_parser.get_audio_errors in 
          data := Some { stream = Id.of_string id
                       ; channel
                       ; pid
                       ; errors
                    };
          Lwt_unix.send_notification notif
      with _ -> ()
    in cb, e

  let of_json conv js =
    Yojson.Safe.from_string js
    |> conv
    |> function Ok v -> v | Error e -> failwith e

  (* TODO change args type to Id * URI *)
  let create args =
    let streams_cb, streams = make_event (of_json Structure.many_of_yojson) in
    let graph_cb, graph = make_event (of_json Structure.many_of_yojson) in
    let wm_cb, wm = make_event (of_json Wm.of_yojson) in
    let vdata_cb, vdata = make_video_event () in
    let adata_cb, adata = make_audio_event () in
    let backend = Qoe_backend.create args
                    ~streams:streams_cb
                    ~graph:graph_cb
                    ~wm:wm_cb
                    ~vdata:vdata_cb
                    ~adata:adata_cb
    and events = { streams
                 ; graph
                 ; wm
                 ; vdata
                 ; adata
                 }
    in backend, events
    

  let run backend =
    Lwt_preemptive.detach Qoe_backend.run backend

  let destroy backend =
    Qoe_backend.free backend

end
