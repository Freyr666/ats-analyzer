open Lwt.Infix

let e, push = Lwt_react.E.create ()

let main () =
  let data = ref "" in
  let notification = Lwt_unix.make_notification (fun () -> push !data) in
  let cb = (fun s ->
      data := s;
      Lwt_unix.send_notification notification)
  in
  Lwt_react.E.keep @@ Lwt_react.E.map_p (fun x -> Lwt_io.printf "Counter: %s\n" x) e;
  Backend.init_logger ();
  let back = Backend.create
               [| "test", "udp://224.1.2.2:1234" |]
               cb
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
