open Rench;

type disposeFunction = unit => unit;

[@deriving show]
type t = {search: (string, list(string) => unit) => disposeFunction};

let process = (rgPath, args, callback) => {
  let cp = ChildProcess.spawn(rgPath, args);

  Event.subscribe(cp.stdout.onData, value => {
    print_endline ("got bytes\n");
    Bytes.to_string(value)
    |> String.trim
    |> String.split_on_char('\n')
    |> callback
  })
  |> ignore;

  () => cp.kill(Sys.sigkill);
};

/**
   Search through files of the directory passed in and sort the results in
   order of the last time they were accessed, alternative sort order includes
   path, modified, created
 */
let search = (path, query, callback) =>
  process(path, [|"--files", "--", query|], callback);

let make = path => {search: search(path)};
