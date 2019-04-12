open Lwt.Infix

let e_streams, push_streams = Lwt_react.E.create ()
let e_graph, push_graph = Lwt_react.E.create ()
let e_wm, push_wm = Lwt_react.E.create ()

let main () =
  let data_streams = ref "" in
  let notif_streams = Lwt_unix.make_notification (fun () -> push_streams !data_streams) in
  let streams_cb = (fun s ->
      data_streams := s;
      Lwt_unix.send_notification notif_streams)
  in
  let data_graph = ref "" in
  let notif_graph = Lwt_unix.make_notification (fun () -> push_graph !data_graph) in
  let graph_cb = (fun s ->
      data_graph := s;
      Lwt_unix.send_notification notif_graph)
  in
  let data_wm = ref "" in
  let notif_wm = Lwt_unix.make_notification (fun () -> push_wm !data_wm) in
  let wm_cb = (fun s ->
      data_wm := s;
      Lwt_unix.send_notification notif_wm)
  in
  Lwt_react.E.keep @@ Lwt_react.E.map_p (fun x -> Lwt_io.printf "Streams: %s\n" x) e_streams;
  Lwt_react.E.keep @@ Lwt_react.E.map_p (fun x -> Lwt_io.printf "Graph: %s\n" x) e_graph;
  Lwt_react.E.keep @@ Lwt_react.E.map_p (fun x -> Lwt_io.printf "Wm: %s\n" x) e_wm;
  Backend.init_logger ();
  let back = Backend.create
               [| "test", "udp://224.1.2.2:1234" |]
               ~streams:streams_cb
               ~graph:graph_cb
               ~wm:wm_cb
               ~vdata:(fun _ _ _ _ -> ())
               ~adata:(fun _ _ _ _ -> ())
  in
  let t = Lwt_preemptive.detach Backend.run back in
  Lwt_unix.sleep 20.0
  >>= fun () ->
  Lwt_io.printf "Streams: %s\n" (Backend.stream_parser_get_structure back)
  >>= fun () ->
  Lwt_unix.sleep 20.0
  >>= fun () ->
  Backend.free back;
  t >>= fun () ->
  Lwt_unix.sleep 20.0

let () =
  Lwt_main.run (main ())
