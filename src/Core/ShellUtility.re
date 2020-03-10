open Kernel;
open Utility;

module Log = (val Log.withNamespace("Oni2.Core.ShellUtility"));

module Constants = {
  let defaultLinuxShell = "/bin/bash";
  let defaultOSXShell = "/bin/zsh";
  let defaultWindowsShell = "powershell.exe";
};

module Internal = {
  let getPathFromEnvironment = () =>
    switch (Sys.getenv_opt("PATH")) {
    | Some(path) => path
    | None =>
      Log.error("Unable to get PATH!");
      "";
    };

  let runCommand = cmd => {
    Log.infof(m => m("Running cmd: %s", cmd));
    let (stdOut, stdIn, stdErr) = Unix.open_process_full(cmd, [||]);
    let ret = input_line(stdOut);
    Log.infof(m => m("Received output: %s", ret));
    let () = close_in(stdOut);
    let () = close_out(stdIn);
    let () = close_in(stdErr);
    ret;
  };

  let getShellFromEnvironment = () => {
    Sys.getenv_opt("SHELL")
    |> OptionEx.flatMap(shellPath => {
         Sys.file_exists(shellPath) ? Some(shellPath) : None
       });
  };

  let discoverLinuxShell = () =>
    try({
      let uid = Unix.getuid();
      let {pw_shell, _}: Unix.passwd_entry = Unix.getpwuid(uid);
      Some(pw_shell);
    }) {
    | ex =>
      Log.warnf(m =>
        m("Unable to get shell from passwd: %s", Printexc.to_string(ex))
      );
      None;
    };

  // This strategy for determing the default shell came from StackOverflow:
  // https://stackoverflow.com/a/41553295
  // This is important because in some cases, like launching from Finder,
  // there may not be an $SHELL environment variable for us.
  let discoverOSXShell = () =>
    // Returns a string of the form: "UserShell: /bin/zsh"
    try({
      let userShell = runCommand("/usr/bin/dscl . read ~/ UserShell");

      let len = String.length(userShell);
      let slashIndex = String.index(userShell, '/');
      Log.infof(m => m("dscl returned: %s", userShell));

      Some(String.sub(userShell, slashIndex, len - slashIndex));
    }) {
    | ex =>
      Log.warnf(m =>
        m("Unable to run dscl to get user shell: %s", Printexc.to_string(ex))
      );
      None;
    };
};

let getDefaultShell = () => {
  (
    lazy({
      let default =
        switch (Revery.Environment.os) {
        | Windows => Constants.defaultWindowsShell
        | Mac => Constants.defaultOSXShell
        | _ => Constants.defaultLinuxShell
        };
      // We assume if the $SHELL environment variable is specified,
      // that should be the default.
      Internal.getShellFromEnvironment()
      |> OptionEx.or_lazy(() => {
           switch (Revery.Environment.os) {
           | Mac => Internal.discoverOSXShell()
           | Linux => Internal.discoverLinuxShell()
           | _ => None
           }
         })
      |> Option.value(~default);
    })
  )
  |> Lazy.force;
};

let getPathFromShell = () => {
  let path =
    switch (Revery.Environment.os) {
    | Mac =>
      let shell = getDefaultShell();
      let shellCmd = Printf.sprintf("%s -lc 'echo $PATH'", shell);
      try(Internal.runCommand(shellCmd)) {
      | ex =>
        Log.warn("Unable to retreive path: " ++ Printexc.to_string(ex));
        Internal.getPathFromEnvironment();
      };
    | _ => Internal.getPathFromEnvironment()
    };

  Log.debug("Path detected as: " ++ path);
  path;
};
