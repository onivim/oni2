open Rench;

type disposeFunction = unit => unit;

[@deriving show]
type t = {search: (string, string, list(string) => unit, unit => unit) => disposeFunction};

let process = (rgPath, args, callback, completedCallback) => {
  let cp = ChildProcess.spawn(rgPath, args);

  let dispose1 =
    Event.subscribe(cp.stdout.onData, value =>
      Revery.App.runOnMainThread(() =>
        Bytes.to_string(value)
        |> String.trim
        |> String.split_on_char('\n')
        |> callback
      )
    );

  let dispose2 = Event.subscribe(cp.onClose, (exitCode) => {
    Log.info("Ripgrep completed - exit code: " ++ string_of_int(exitCode));
    completedCallback();
  });

  () => {
    dispose1();
    dispose2();
    cp.kill(Sys.sigkill);
  };
};

/**
   Search through files of the directory passed in and sort the results in
   order of the last time they were accessed, alternative sort order includes
   path, modified, created
 */
let search = (path, search, workingDirectory, callback, completedCallback) =>
  process(path, [|"--smart-case", "--files", "-g", search, "!_esy/*", "!node_modules/*", "--", workingDirectory|], callback, completedCallback);

let make = path => {search: search(path)};
