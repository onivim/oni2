open Kernel;

module Log = (val Log.withNamespace("Oni2.Core.ShellUtility"));

module Internal = {
  let defaultPosixShell = "/bin/sh";
  
  let getPathFromEnvironment = () =>
    switch (Sys.getenv_opt("PATH")) {
    | Some(path) => path
    | None =>
      Log.error("Unable to get PATH!");
      "";
    };

  let runCommand = (cmd) => {
      Log.infof(m => m("Running cmd: %s", cmd));
      let (stdOut, stdIn, stdErr) =
        Unix.open_process_full(cmd, [||]);
      let ret = input_line(stdOut);
      Log.infof(m => m("Received output: %s", ret));
      let () = close_in(stdOut);
      let () = close_out(stdIn);
      let () = close_in(stdErr);
      ret;
  };

  let discoverLinuxShell = () =>  {
    try({
      let user = Sys.getenv("USER");
      let userShell = runCommand("getent passwd " ++ user ++ " | awk -F: '{print $NF}'");
      userShell
    }) {
    | ex =>
      Log.warn("Unable to get shell from getent");
      defaultPosixShell
    }
  };

  // This strategy for determing the default shell came from StackOverflow:
  // https://stackoverflow.com/a/41553295
  // This is important because in some cases, like launching from Finder,
  // there may not be an $SHELL environment variable for us.
  let discoverOSXShell = () =>
    try({
      // Returns a string of the form: "UserShell: /bin/zsh"
      let userShell = runCommand("/usr/bin/dscl . read ~/ UserShell")

      let len = String.length(userShell);
      let slashIndex = String.index(userShell, '/');
      Log.infof(m => m("dscl returned: %s", userShell));

      String.sub(userShell, slashIndex, len - slashIndex);
    }) {
    | ex =>
      Log.warn("Unable to run dscl to get user shell");
      defaultPosixShell
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
          let shell = switch (Revery.Environment.os) {
          | Windows => "cmd.exe"
          | Mac => Internal.discoverOSXShell()
          | Linux => Internal.discoverLinuxShell()
          | _ => "/bin/sh"
          };
          prerr_endline ("SHELL: " ++ shell);
          shell
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
        Internal.runCommand(shellCmd);
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
