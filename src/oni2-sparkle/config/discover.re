module Configurator = Configurator.V1;
open Configurator.C_define;

let projectRoot = Sys.getenv("ONI2_ROOT");
let sparkleDir = projectRoot ++ "/vendor/Sparkle-1.26.0/";
let winSparkleDir = projectRoot ++ "/vendor/WinSparkle-0.7.0/";

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

let useSparkle =
  switch (os) {
  | Mac =>
    switch (Sys.getenv_opt("ONI2_BUILD_MODE")) {
    | Some("Release") => true
    | Some(_)
    | None => false
    }
  | _ => false
  };

let useWinSparkle =
  switch (os) {
  | Windows =>
    switch (Sys.getenv_opt("ONI2_BUILD_MODE")) {
    | Some("Release") => true
    | Some(_)
    | None => false
    }
  | _ => false
  };

let flags =
  switch (os) {
  | Mac =>
    []
    @ cclib("-framework Foundation")
    @ (
      useSparkle
        ? []
          @ ccopt("-rpath @executable_path/../Frameworks")
          @ cclib("-framework Sparkle")
          @ ccopt("-F " ++ sparkleDir)
        : []
    )
  | Windows =>
    []
    @ (
      useWinSparkle
        ? []
          @ ccopt("-L " ++ winSparkleDir ++ "/x64/Release/")
          @ cclib("-lwinsparkle")
          @ cclib("-lkernel32")
          @ cclib("-lshlwapi")
        : []
    )
  | _ => []
  };

let cFlags =
  switch (os) {
  | Mac => ["-F", sparkleDir, "-x", "objective-c"]
  | Windows => ["-I", winSparkleDir ++ "/include/"]
  | _ => []
  };

let generateHeaderFile = conf => {
  let enableAutoUpdate =
    useWinSparkle || useSparkle ? Value.Int(1) : Value.Switch(false);
  let useSparkle = useSparkle ? Value.Int(1) : Value.Switch(false);
  let useWinSparkle = useWinSparkle ? Value.Int(1) : Value.Switch(false);
  gen_header_file(
    conf,
    [
      ("USE_SPARKLE", useSparkle),
      ("USE_WIN_SPARKLE", useWinSparkle),
      ("ENABLE_AUTOUPDATE", enableAutoUpdate),
    ],
  );
};

Configurator.main(~name="discover", t => {
  generateHeaderFile(~fname="config.h", t);
  Configurator.Flags.write_sexp("c_flags.sexp", cFlags);
  Configurator.Flags.write_sexp("flags.sexp", flags);
});
