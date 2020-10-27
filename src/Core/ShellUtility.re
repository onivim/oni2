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

  let environmentLinesToMap = (hasDelimiter, lines) => {
    let rec loop = (hasEncounteredDelimiter, acc, lines) => {
      switch (lines) {
      | [] => acc
      | [line, ...lines] =>
        if (!hasEncounteredDelimiter) {
          if (StringEx.contains("_SHELL_ENV_DELIMITER_", line)) {
            loop(true, acc, lines);
          } else {
            loop(false, acc, lines);
          };
        } else if (StringEx.contains("_SHELL_ENV_DELIMITER_", line)) {
          acc;
        } else if (!StringEx.isEmpty(line)) {
          loop(true, [line, ...acc], lines);
        } else {
          loop(true, acc, lines);
        }
      };
    };

    let prunedLines = loop(!hasDelimiter, [], lines);
    prunedLines
    |> List.fold_left(
         (acc, curr) => {
           switch (String.split_on_char('=', curr)) {
           | [] => acc
           | [_] => acc
           | [key, ...values] =>
             let v = String.concat("=", values);
             StringMap.add(key, v, acc);
           }
         },
         StringMap.empty,
       );
  };

  let getDefaultShellEnvironment = shellCmd => {
    switch (Revery.Environment.os) {
    // Linux and Mac - use `env` command, and parse using a strategy from shell-env:
    // https://github.com/sindresorhus/shell-env/blob/a95fd441d3b7cc2c122899de72da30dae70936ec/index.js#L6
    | Linux
    | Mac =>
      let args = [
        "-ilc",
        "printf \"\n_SHELL_ENV_DELIMITER_\"; env; printf \"\n_SHELL_ENV_DELIMITER_\n\"; exit",
      ];
      let (inp, out, err) =
        Unix.open_process_args_full(
          shellCmd,
          [shellCmd, ...args] |> Array.of_list,
          [||],
        );
      let lines = ref([]);

      let outLines =
        try(
          {
            while (true) {
              lines := [input_line(inp), ...lines^];
            };
            lines^;
          }
        ) {
        | End_of_file =>
          close_in(inp);
          lines^;
        };

      close_out(out);
      close_in(err);
      environmentLinesToMap(true, outLines);

    // Windows - just use default environment.
    | Windows
    | _ => Unix.environment() |> Array.to_list |> environmentLinesToMap(false)
    };
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

let getDefaultShellEnvironment = () => {
  (
    lazy({
      let shellCmd = getDefaultShell();
      Internal.getDefaultShellEnvironment(shellCmd);
    })
  )
  |> Lazy.force;
};

let getPathFromEnvironment = Internal.getPathFromEnvironment;

let getPathFromShell = () => {
  let path =
    switch (Revery.Environment.os) {
    | Mac =>
      let envMap = getDefaultShellEnvironment();
      switch (StringMap.find_opt("PATH", envMap)) {
      | None =>
        Log.warn("Unable to retreive path from env command.");
        Internal.getPathFromEnvironment();
      | Some(path) => path
      };
    | _ => Internal.getPathFromEnvironment()
    };

  Log.debug("Path detected as: " ++ path);
  path;
};

let fixOSXPath = () =>
  // #1161 - OSX - Make sure we're using the terminal / shell PATH.
  // By default, if the application is opened via Finder, it won't get the shell $PATH.
  // Use a strategy like the fix-path NPM module:
  // https://github.com/sindresorhus/fix-path/blob/8f12bec72ee4319638a43758b40d61c25427404b/index.js#L5
  if (Revery.Environment.os == Mac) {
    let shellEnv = getDefaultShellEnvironment();
    switch (StringMap.find_opt("PATH", shellEnv)) {
    | Some(path) =>
      Log.infof(m => m("OSX - setting path from shell: %s", path));
      Luv.Env.setenv(~value=path, "PATH")
      |> Utility.ResultEx.tapError(err => {
           Log.errorf(m =>
             m("Unable to set PATH: %s", Luv.Error.strerror(err))
           )
         })
      |> Result.iter(() => Log.info("Set PATH successfully"));
    | None => Log.warn("OSX - Unable to get shell path.")
    };
  };
