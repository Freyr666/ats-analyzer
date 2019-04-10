open Lwt.Infix

let main () =
  Backend.init_logger ();
  let back = Backend.create [| "test", "udp://224.1.2.2:1234" |] in
  let t = Lwt_preemptive.detach Backend.run back in
  Lwt_unix.sleep 20.0
  >>= fun () ->
  Lwt_io.printf "Streams: %s\n" (Backend.stream_parser_get_structure back)
  >>= fun () ->
  Lwt_unix.sleep 20.0
  >>= fun () ->
  Backend.free back;
  t >>= fun () ->
  Lwt_io.printf "Streams 2: %s\n" (Backend.stream_parser_get_structure back)

let () =
  Lwt_main.run (main ())
