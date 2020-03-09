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

  // This strategy for determing the default shell came from StackOverflow:
  // https://stackoverflow.com/a/41553295
  // This is important because in some cases, like launching from Finder,
  // there may not be an $SHELL environment variable for us.
  let discoverOSXShell = () =>
    try({
      let (stdOut, stdIn, stdErr) =
        Unix.open_process_full("/usr/bin/dscl . read ~/ UserShell", [||]);
      let userShell = input_line(stdOut);
      let () = close_in(stdOut);
      let () = close_out(stdIn);
      let () = close_in(stdErr);
      // Returns a string of the form: "UserShell: /bin/zsh"

      let len = String.length(userShell);
      let slashIndex = String.index(userShell, '/');
      Log.infof(m => m("dscl returned: %s", userShell));

      String.sub(userShell, slashIndex, len - slashIndex);
    }) {
    | ex =>
      Log.warn("Unable to run dscl to get user shell");
      "/bin/bash";
    };
};

let getDefaultShell = () => {
  (
    lazy(
      {
        // We assume if the $SHELL environment variable is specified,
        // that should be the default.
        switch (Sys.getenv_opt("SHELL")) {
        | Some(v) => v
        | None =>
          switch (Revery.Environment.os) {
          | Windows => "powershell.exe"
          | Mac => Internal.discoverOSXShell()
          | _ => "/bin/bash"
          }
        };
      }
    )
  )
  |> Lazy.force;
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
