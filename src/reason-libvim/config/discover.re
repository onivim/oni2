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

let getLibIntlPath = () =>
  try({
    let ic =
      Unix.open_process_in(
        "find /usr/local/Cellar -name libintl.a -print 2>/dev/null",
      );
    let path = input_line(ic);
    let () = close_in(ic);
    path;
  }) {
  | ex =>
    prerr_endline("Unable to find libintl: " ++ Printexc.to_string(ex));
    prerr_endline("You may need to install gettext: `brew install gettext`");
    failwith("Unable to find libintl");
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

let root = Sys.getenv("cur__root");
let libvimIncludePath = Sys.getenv("LIBVIM_INCLUDE_PATH");
let libvimLibPath = Sys.getenv("LIBVIM_LIB_PATH");
let c_flags = [
  "-I",
  libvimIncludePath,
  "-I",
  Filename.concat(libvimIncludePath, "proto"),
]; /* "-I"; Filename.concat root "src"] */

let c_flags =
  switch (get_os) {
  | Windows => c_flags
  | Linux => c_flags @ ["-DHAVE_CONFIG_H"]
  | _ => c_flags @ ["-DHAVE_CONFIG_H", "-DMACOS_X", "-DMACOS_X_DARWIN"]
  };

let libPath = "-L" ++ Sys.getenv("LIBVIM_LIB_PATH");

let _ = print_endline(libPath);

let ccopt = s => ["-ccopt", s];
let cclib = s => ["-cclib", s];

let flags =
  switch (get_os) {
  | Windows =>
    []
    @ ccopt(libPath)
    @ cclib("-lvim")
    @ cclib("-luuid")
    @ cclib("-lnetapi32")
    @ cclib("-lole32")
    @ cclib("-lgdi32")
  | Linux =>
    []
    @ ccopt(libPath)
    @ cclib("-lvim")
    @ cclib("-lacl")
    @ cclib("-lICE")
    @ cclib("-lX11")
    @ cclib("-lSM")
    @ cclib("-lncurses")
    @ cclib("-lXt")
  | _ =>
    []
    @ ccopt(libPath)
    @ cclib(getLibIntlPath())
    @ cclib("-lvim")
    @ cclib("-lm")
    @ cclib("-lncurses")
    @ cclib("-liconv")
    @ cclib("-framework AppKit")
  };

let flags_with_sanitize =
  switch (get_os) {
  | Linux => flags @ ccopt("-fsanitize=address")
  | _ => flags
  };

let cxx_flags =
  switch (get_os) {
  | Windows => c_flags @ ["-fno-exceptions", "-fno-rtti", "-lstdc++"]
  | _ => c_flags
  };

{
  Configurator.V1.Flags.write_sexp("c_flags.sexp", c_flags);
  Configurator.V1.Flags.write_sexp("cxx_flags.sexp", cxx_flags);
  Configurator.V1.Flags.write_sexp("flags.sexp", flags);
  Configurator.V1.Flags.write_sexp(
    "flags_with_sanitize.sexp",
    flags_with_sanitize,
  );
};
