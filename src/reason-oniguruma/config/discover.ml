
type os =
    | Windows
    | Mac
    | Linux
    | Unknown

let uname () =
    let ic = Unix.open_process_in "uname" in
    let uname = input_line ic in
    let () = close_in ic in
    uname;;

let get_os =
    match Sys.os_type with
    | "Win32" -> Windows
    | _ -> match uname () with
        | "Darwin" -> Mac
        | "Linux" -> Linux
        | _ -> Unknown

let onigurumaIncludePath = Sys.getenv "ONIGURUMA_INCLUDE_PATH"
let onigurumaLibPath = Sys.getenv "ONIGURUMA_LIB_PATH"
let c_flags = ["-I"; onigurumaIncludePath; "-I"; onigurumaLibPath ]

let _ = print_endline (onigurumaIncludePath)
let _ = print_endline (onigurumaLibPath)

let ccopt s = ["-ccopt"; s]
let cclib s = ["-cclib"; s]

let libPath = "-L" ^ onigurumaLibPath

let flags = []
        @ ccopt(libPath)
        @ cclib("-lonig")
;;

let flags_with_sanitize =
    match get_os with
    (* There is a known leak tracked here:
       https://github.com/kkos/oniguruma/issues/31 
       TODO: Investigate upgrading to bring back ASAN for this lib *)
    (*| Linux -> flags @ ccopt("-fsanitize=address")*)
    | _ -> flags
;;

let cxx_flags = c_flags
;;

Configurator.V1.Flags.write_sexp "c_flags.sexp" c_flags;
Configurator.V1.Flags.write_sexp "cxx_flags.sexp" cxx_flags;
Configurator.V1.Flags.write_sexp "flags.sexp" flags;
Configurator.V1.Flags.write_sexp "flags_with_sanitize.sexp" flags_with_sanitize;
