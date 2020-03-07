open Kernel;

module Log = (val Log.withNamespace("Oni2.Core.ShellUtility"));

module Internal = {
  let getPathFromEnvironment = () =>
    switch (Sys.getenv_opt("PATH")) {
    | Some(path) => path
    | None =>
      Log.error("Unable to get PATH!");
      "";
    };
};

let getDefaultShell = () => {
  switch (Sys.getenv_opt("SHELL")) {
  | Some(v) => v
  | None => Sys.win32 ? "cmd.exe" : "/bin/bash"
  };
};

let getPathFromShell = () => {
  let path =
    switch (Revery.Environment.os) {
    | Mac =>
      let shell = getDefaultShell();
      let shellCmd = Printf.sprintf("%s -lc 'echo $PATH'", shell);
      try({
        let (stdOut, stdIn, stdErr) = Unix.open_process_full(shellCmd, [||]);
        let path = input_line(stdOut);
        let () = close_in(stdOut);
        let () = close_out(stdIn);
        let () = close_in(stdErr);
        path;
      }) {
      | ex =>
        Log.warn("Unable to retreive path: " ++ Printexc.to_string(ex));
        Internal.getPathFromEnvironment();
      };
    | _ => Internal.getPathFromEnvironment()
    };

  Log.debug("Path detected as: " ++ path);
  path;
};
