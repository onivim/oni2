open Utility;

module Log = (val Log.withNamespace("Oni2.ShellUtility"));

module Internal = {
  let getDefaultShell = () => {
    switch (Sys.getenv_opt("SHELL")) {
    | Some(v) => v
    | None => Sys.win32 ? "cmd.exe" : "/usr/bin/bash"
    };
  };

  let getShellFromPath = () => {
    Sys.getenv_opt("PATH")
    |> Option.tap_none(() => Log.error("Unable to get PATH!"))
    |> Option.value(~default="");
  };
};

let getShellPath = () => {
  let res =
    switch (Revery.Environment.os) {
    | Mac =>
      let shell = Internal.getDefaultShell();
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
        Log.error("Unable to retrive path: " ++ Printexc.to_string(ex));
        Internal.getShellFromPath();
      };
    | _ => Internal.getShellFromPath()
    };

  Log.info("Path is: " ++ res);
  res;
};
