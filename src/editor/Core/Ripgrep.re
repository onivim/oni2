open Rench;

type disposeFunction = unit => unit;

/* Internal counters used for tracking */
let _ripGrepRunCount = ref(0);
let _ripGrepCompletedCount = ref(0);

let getRunCount = () => _ripGrepRunCount^;
let getCompletedCount = () => _ripGrepCompletedCount^;

[@deriving show]
type t = {
  search:
    (string, string, list(string) => unit, unit => unit) => disposeFunction,
};
        
let isNotDirectory = item =>
  switch (Sys.is_directory(item)) {
  | exception _ => false
  | v => !v
  };

let process = (workingDirectory, rgPath, args, callback, completedCallback) => {
  incr(_ripGrepRunCount);
  let argsStr = String.concat("|", Array.to_list(args));
  Log.info("[Ripgrep] Starting process: " ++ rgPath ++ " with args: |" ++ argsStr ++ "|");
  let cp = ChildProcess.spawn(~cwd=Some(workingDirectory), rgPath, args);

  let dispose1 =
    Event.subscribe(
      cp.stdout.onData,
      value => {
        let _ =
          Thread.create(
            v => {
              Log.debug("[Ripgrep] Processing bytes: " ++ string_of_int(Bytes.length(v)));
              let items =
                Bytes.to_string(v)
                |> String.trim
                |> String.split_on_char('\n')
                |> List.filter(isNotDirectory);
              
              Log.debug("[Ripgrep] Processing bytes completed");
              Revery.App.runOnMainThread(() => callback(items));
            },
            value,
          );
        ();
      },
    );

  let dispose2 =
    Event.subscribe(
      cp.onClose,
      exitCode => {
        incr(_ripGrepCompletedCount);
        Log.info(
          "[Ripgrep] Completed - exit code: " ++ string_of_int(exitCode),
        );
        completedCallback();
      },
    );

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
let search = (path, search, workingDirectory, callback, completedCallback) => {
  process(
    workingDirectory,
    path,
    [|
      "--smart-case",
      "--files",
    |],
    callback,
    completedCallback,
  );
};

let make = path => {search: search(path)};
