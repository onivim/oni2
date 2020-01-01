open Utility;

module Log = (val Log.withNamespace("Oni2.ShellUtility"));

module Internal = {
  let getDefaultShell = () => {
    switch (Sys.getenv_opt("SHELL")) {
    | Some(v) => v
    | None => Sys.win32 ? "cmd.exe" : "/usr/bin/bash"
    };
  };
};

let getShellPath = () => {
  let res =
    switch (Revery.Environment.os) {
    | Mac =>
      let shell = Internal.getDefaultShell();
      let shellCmd = Printf.sprintf("%s -lc 'echo $PATH'", shell);
      let ic = Unix.open_process_in(shellCmd);
      let path = input_line(ic);
      let () = close_in(ic);
      path;
    | _ =>
      Sys.getenv_opt("PATH")
      |> Option.tap_none(() => Log.error("Unable to get PATH!"))
      |> Option.value(~default="")
    };

  Log.info("Path is: " ++ res);
  res;
};
