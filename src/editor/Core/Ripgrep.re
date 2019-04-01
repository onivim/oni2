open Rench;

[@deriving show]
type t = {search: (string, list(string) => unit) => unit};

let process = (rgPath, args, callback) => {
  let cp = ChildProcess.spawn(rgPath, args);

  Event.subscribe(cp.stdout.onData, value =>
    Bytes.to_string(value)
    |> String.trim
    |> String.split_on_char('\n')
    |> callback
  )
  |> ignore;
};

/**
   Search through files of the directory passed in and sort the results in
   order of the last time they were accessed, alternative sort order includes
   path, modified, created
 */
let search = (path, query, callback) =>
  process(path, [|"--files", "--sort", "accessed", "--", query|], callback);

let make = path => {search: search(path)};
