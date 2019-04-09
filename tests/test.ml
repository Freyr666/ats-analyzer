open Lwt.Infix

let main () =
  Backend.init_logger ();
  let back = Backend.create () in
  let t = Lwt_preemptive.detach Backend.run back in
  Lwt_unix.sleep 20.0
  >>= fun () ->
  Lwt_io.printf "Streams: %s\n" (Backend.get_streams back)
  >>= fun () ->
  Lwt_unix.sleep 20.0
  >>= fun () ->
  Backend.quit back;
  t

let () =
  Lwt_main.run (main ())
