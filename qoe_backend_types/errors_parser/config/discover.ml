module C = Configurator.V1

let () =
  C.main ~name:"backend" (fun c ->
      let default : C.Pkg_config.package_conf =
        { libs   = []
        ; cflags = [ "-I/usr/include/gstreamer-1.0"
                   ; "-I/usr/include/glib-2.0"
                   ; "-I/usr/lib/glib-2.0/include"
                   ]
        }
      in
      let conf =
        match C.Pkg_config.get c with
        | None -> default
        | Some pc ->
           match (C.Pkg_config.query pc ~package:"gstreamer-1.0") with
           | None -> default
           | Some deps -> deps
      in

      C.Flags.write_sexp "c_flags.sexp"         conf.cflags)
