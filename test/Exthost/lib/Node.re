open Oni_Core;
module Log = (val Timber.Log.withNamespace("ExtHost.TestLib.Node"));

let spawn = (~env=[], ~onExit, args) => {
  let nodeFullPath = Setup.init().nodePath;
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
