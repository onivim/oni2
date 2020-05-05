module Log = (val Timber.Log.withNamespace("ExtHost.TestLib.Node"));

let getNodePath = () => {
  let ic =
    Sys.win32
      // HACK: Not sure why this command doesn't work on Linux / macOS, and vice versa...
      ? Unix.open_process_args_in(
          "node",
          [|"node", "-e", "console.log(process.execPath)"|],
        )
      : Unix.open_process_in("node -e 'console.log(process.execPath)'");
  let nodePath = input_line(ic);
  let _ = close_in(ic);
  nodePath |> String.trim;
};

let spawn = (~env=[], ~onExit, args) => {
  let nodeFullPath = getNodePath();
  Log.info("Using node path: " ++ nodeFullPath);
  Luv.Process.spawn(
    ~on_exit=onExit,
    ~environment=env,
    ~redirect=[
      Luv.Process.inherit_fd(
        ~fd=Luv.Process.stdin,
        ~from_parent_fd=Luv.Process.stdin,
        (),
      ),
      Luv.Process.inherit_fd(
        ~fd=Luv.Process.stdout,
        ~from_parent_fd=Luv.Process.stderr,
        (),
      ),
      Luv.Process.inherit_fd(
        ~fd=Luv.Process.stderr,
        ~from_parent_fd=Luv.Process.stderr,
        (),
      ),
    ],
    nodeFullPath,
    [nodeFullPath, ...args],
  )
  |> ResultEx.tap_error(msg => prerr_endline(Luv.Error.strerror(msg)))
  |> Result.get_ok;
};
