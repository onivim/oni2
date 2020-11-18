module Configurator = Configurator.V1;

let ccopt = s => ["-ccopt", s];
let cclib = s => ["-cclib", s];

type os =
  | Windows
  | Mac
  | Linux
  | Unknown;

let os =
  switch (Sys.os_type) {
  | "Win32" => Windows
  | _ =>
    let ic = Unix.open_process_in("uname");
    let uname = input_line(ic);
    let _ = close_in(ic);
    switch (uname) {
    | "Darwin" => Mac
    | "Linux" => Linux
    | _ => Unknown
    };
  };

let flags =
  switch (os) {
  | Mac => [] @ cclib("-framework AppKit")
  | Linux => [] @ cclib("-lX11") @ cclib("-lxkbfile")
  | _ => []
  };

let cFlags =
  switch (os) {
  | Mac => ["-I", Sys.getenv("SDL2_INCLUDE_PATH"), "-x", "objective-c"]
  | _ => ["-I", Sys.getenv("SDL2_INCLUDE_PATH")]
  };

Configurator.main(~name="discover", _ => {
  Configurator.Flags.write_sexp("c_flags.sexp", cFlags);
  Configurator.Flags.write_sexp("flags.sexp", flags);
});
