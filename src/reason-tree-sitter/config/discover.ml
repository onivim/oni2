
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

let treeSitterIncludePath = Sys.getenv "TREESITTER_INCLUDE_PATH"
let treeSitterLibPath = Sys.getenv "TREESITTER_LIB_PATH"
let c_flags = ["-Werror"; "-I"; treeSitterIncludePath; "-I"; treeSitterLibPath ]

let _ = print_endline (treeSitterIncludePath)
let _ = print_endline (treeSitterLibPath)

let ccopt s = ["-ccopt"; s]
let cclib s = ["-cclib"; s]

let libPath = "-L" ^ treeSitterLibPath

let flags = []
        @ ccopt(libPath)
        @ cclib("-ltree-sitter")
;;

let flags_with_sanitize =
    match get_os with
    | Linux -> flags @ ccopt("-fsanitize=address")
    | _ -> flags
;;

let cxx_flags = c_flags
;;

Configurator.V1.Flags.write_sexp "c_flags.sexp" c_flags;
Configurator.V1.Flags.write_sexp "cxx_flags.sexp" cxx_flags;
Configurator.V1.Flags.write_sexp "flags.sexp" flags;
Configurator.V1.Flags.write_sexp "flags_with_sanitize.sexp" flags_with_sanitize;
