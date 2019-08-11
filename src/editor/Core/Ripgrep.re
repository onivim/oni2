open Rench;

type disposeFunction = unit => unit;

[@deriving show]
type t = {search: (string, list(string) => unit) => disposeFunction};

let process = (rgPath, args, callback, done) => {
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

  let dispose2 = Event.subscribe(cp.onClose, () => {
    Log.info("Ripgrep completed.");
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
let search = (path, query, callback, done) =>
  process(path, [|"--files", "--", query|], callback, done);

let make = path => {search: search(path)};
