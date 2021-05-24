open Configurator.V1.Flags;
module C = Configurator.V1;

type os =
  | Windows
  | Mac
  | Linux
  | Unknown;

let uname = () => {
  let ic = Unix.open_process_in("uname");
  let uname = input_line(ic);
  let () = close_in(ic);
  uname;
};

let get_os =
  switch (Sys.os_type) {
  | "Win32" => Windows
  | _ =>
    switch (uname()) {
    | "Darwin" => Mac
    | "Linux" => Linux
    | _ => Unknown
    }
  };

let () = {
  let ccopt = s => ["-ccopt", s];
  let flags = [];

  let cflags =
    switch (get_os) {
    | _ => ["-Wall", "-Werror", "-I" ++ Sys.getenv("LIBVTERM_INCLUDE_PATH")]
    };

  let libs =
    switch (get_os) {
    | _ => [Sys.getenv("LIBVTERM_LIB_PATH") ++ "/libvterm.a"]
    };

  let flags_with_sanitize =
    switch (get_os) {
    | Linux => flags @ ccopt("-fsanitize=address")
    | _ => flags
    };

  write_sexp("flags.sexp", flags);
  write_sexp("flags_with_sanitize.sexp", flags_with_sanitize);
  write_lines("c_flags.txt", cflags);
  write_sexp("c_flags.sexp", cflags);
  write_sexp("c_library_flags.sexp", libs);
  write_lines("c_library_flags.txt", libs);
  write_sexp(
    "cclib_c_library_flags.sexp",
    libs |> List.map(o => ["-cclib", o]) |> List.flatten,
  );
};
