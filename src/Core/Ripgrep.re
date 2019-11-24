open Rench;

module Time = Revery_Core.Time;

module List = Utility.List;

type disposeFunction = unit => unit;

/* Internal counters used for tracking */
let _ripGrepRunCount = ref(0);
let _ripGrepCompletedCount = ref(0);

let getRunCount = () => _ripGrepRunCount^;
let getCompletedCount = () => _ripGrepCompletedCount^;

type searchFunction =
  (string, list(string) => unit, unit => unit) => disposeFunction;

module Match = {
  type t = {
    file: string,
    text: string,
    lineNumber: int,
    charStart: int,
    charEnd: int,
  };

  let fromJsonString = str => {
    Yojson.Basic.Util.(
      try({
        let json = Yojson.Basic.from_string(str);

        if (member("type", json) == `String("match")) {
          let data = member("data", json);

          let submatches = data |> member("submatches") |> to_list;

          let matches =
            submatches
            |> List.map(submatch =>
                 {
                   file:
                     data |> member("path") |> member("text") |> to_string,
                   text:
                     data |> member("lines") |> member("text") |> to_string,
                   lineNumber: data |> member("line_number") |> to_int,
                   charStart: submatch |> member("start") |> to_int,
                   charEnd: submatch |> member("end") |> to_int,
                 }
               );

          Some(matches);
        } else {
          None; // Not a "match" message
        };
      }) {
      | Type_error(message, _) =>
        Log.error("[Ripgrep.Match] Error decoding JSON: " ++ message);
        None;
      | Yojson.Json_error(message) =>
        Log.error("[Ripgrep.Match] Error parsing JSON: " ++ message);
        None;
      }
    );
  };
};

type t = {
  search: searchFunction,
  findInFiles:
    (string, string, list(Match.t) => unit, unit => unit) => disposeFunction,
};

/**
 RipgrepProcessingJob is the logic for processing a [Bytes.t]
 and sending it back to whatever is listening to Ripgrep.

 Each iteration of work, it will split up a [Bytes.t] (a chunk of output),
 get a list of files, and send that to the callback.
*/
module RipgrepProcessingJob = {
  type pendingWork = {
    duplicateHash: Hashtbl.t(string, bool),
    callback: list(string) => unit,
    bytes: list(Bytes.t),
  };

  let pendingWorkPrinter = (p: pendingWork) => {
    "Byte chunks left: " ++ string_of_int(List.length(p.bytes));
  };

  type t = Job.t(pendingWork, unit);

  let dedup = (hash, str) => {
    switch (Hashtbl.find_opt(hash, str)) {
    | Some(_) => false
    | None =>
      Hashtbl.add(hash, str, true);
      true;
    };
  };

  let doWork = (pendingWork, c) => {
    let newBytes =
      switch (pendingWork.bytes) {
      | [] => []
      | [hd, ...tail] =>
        let items =
          hd
          |> Bytes.to_string
          |> String.trim
          |> String.split_on_char('\n')
          |> List.filter(dedup(pendingWork.duplicateHash));
        pendingWork.callback(items);
        tail;
      };

    let isDone =
      switch (newBytes) {
      | [] => true
      | _ => false
      };

    (isDone, {...pendingWork, bytes: newBytes}, c);
  };

  let create = (~callback, ()) => {
    let duplicateHash = Hashtbl.create(1000);
    Job.create(
      ~f=doWork,
      ~initialCompletedWork=(),
      ~name="RipgrepProcessorJob",
      ~pendingWorkPrinter,
      ~budget=Time.ms(2),
      {callback, bytes: [], duplicateHash},
    );
  };

  let queueWork = (bytes: Bytes.t, currentJob: t) => {
    Job.map(
      (p, c) => {
        let newP: pendingWork = {...p, bytes: [bytes, ...p.bytes]};
        (false, newP, c);
      },
      currentJob,
    );
  };
};

let process = (rgPath, args, callback, completedCallback) => {
  incr(_ripGrepRunCount);
  let argsStr = String.concat("|", Array.to_list(args));
  Log.info(
    "[Ripgrep] Starting process: "
    ++ rgPath
    ++ " with args: |"
    ++ argsStr
    ++ "|",
  );
  // Mutex to
  let jobMutex = Mutex.create();
  let job = ref(RipgrepProcessingJob.create(~callback, ()));

  let dispose3 = ref(None);

  Revery.App.runOnMainThread(() =>
    dispose3 :=
      Some(
        Revery.Tick.interval(
          _ =>
            if (!Job.isComplete(job^)) {
              Mutex.lock(jobMutex);
              job := Job.tick(job^);
              Mutex.unlock(jobMutex);
            },
          Time.zero,
        ),
      )
  );

  let cp = ChildProcess.spawn(rgPath, args);

  let dispose1 =
    Event.subscribe(
      cp.stdout.onData,
      value => {
        Mutex.lock(jobMutex);
        job := RipgrepProcessingJob.queueWork(value, job^);
        Mutex.unlock(jobMutex);
      },
    );

  let dispose2 =
    Event.subscribe(
      cp.onClose,
      exitCode => {
        incr(_ripGrepCompletedCount);
        Log.info(
          "[Ripgrep] Process completed - exit code: "
          ++ string_of_int(exitCode),
        );
        completedCallback();
      },
    );

  () => {
    Log.info("Ripgrep session complete.");
    dispose1();
    dispose2();
    switch (dispose3^) {
    | Some(v) => v()
    | None => ()
    };
    cp.kill(Sys.sigkill);
  };
};

/**
   Search through files of the directory passed in and sort the results in
   order of the last time they were accessed, alternative sort order includes
   path, modified, created
 */
let search = (path, workingDirectory, callback, completedCallback) => {
  process(
    path,
    [|"--smart-case", "--files", "--", workingDirectory|],
    callback,
    completedCallback,
  );
};

module RipgrepJsonProcessingJob = {
  type pendingWork = {
    callback: list(Match.t) => unit,
    bytes: list(Bytes.t),
  };

  let pendingWorkPrinter = (p: pendingWork) => {
    "Byte chunks left: " ++ string_of_int(List.length(p.bytes));
  };

  type t = Job.t(pendingWork, unit);

  let doWork = (pendingWork, c) => {
    let newBytes =
      switch (pendingWork.bytes) {
      | [] => []
      | [hd, ...tail] =>
        let items =
          hd
          |> Bytes.to_string
          |> String.trim
          |> String.split_on_char('\n')
          |> List.filter_map(Match.fromJsonString)
          |> List.concat;
        pendingWork.callback(items);
        tail;
      };

    let isDone =
      switch (newBytes) {
      | [] => true
      | _ => false
      };

    (isDone, {...pendingWork, bytes: newBytes}, c);
  };

  let create = (~callback, ()) => {
    Job.create(
      ~f=doWork,
      ~initialCompletedWork=(),
      ~name="RipgrepProcessorJob",
      ~pendingWorkPrinter,
      ~budget=Time.ms(2),
      {callback, bytes: []},
    );
  };

  let queueWork = (bytes: Bytes.t, currentJob: t) => {
    Job.map(
      (p, c) => {
        let newP: pendingWork = {...p, bytes: [bytes, ...p.bytes]};
        (false, newP, c);
      },
      currentJob,
    );
  };
};

let processJson = (rgPath, args, callback, completedCallback) => {
  incr(_ripGrepRunCount);
  let argsStr = String.concat("|", Array.to_list(args));
  Log.info(
    "[Ripgrep] Starting process: "
    ++ rgPath
    ++ " with args: |"
    ++ argsStr
    ++ "|",
  );
  // Mutex to
  let jobMutex = Mutex.create();
  let job = ref(RipgrepJsonProcessingJob.create(~callback, ()));

  let dispose3 = ref(None);

  Revery.App.runOnMainThread(() =>
    dispose3 :=
      Some(
        Revery.Tick.interval(
          _ =>
            if (!Job.isComplete(job^)) {
              Mutex.lock(jobMutex);
              job := Job.tick(job^);
              Mutex.unlock(jobMutex);
            },
          Time.zero,
        ),
      )
  );

  let cp = ChildProcess.spawn(rgPath, args);

  let dispose1 =
    Event.subscribe(
      cp.stdout.onData,
      value => {
        Mutex.lock(jobMutex);
        job := RipgrepJsonProcessingJob.queueWork(value, job^);
        Mutex.unlock(jobMutex);
      },
    );

  let dispose2 =
    Event.subscribe(
      cp.onClose,
      exitCode => {
        incr(_ripGrepCompletedCount);
        Log.info(
          "[Ripgrep] Process completed - exit code: "
          ++ string_of_int(exitCode),
        );
        completedCallback();
      },
    );

  () => {
    Log.info("Ripgrep session complete.");
    dispose1();
    dispose2();
    switch (dispose3^) {
    | Some(v) => v()
    | None => ()
    };
    cp.kill(Sys.sigkill);
  };
};

let findInFiles = (path, workingDirectory, query, callback, completedCallback) => {
  processJson(
    path,
    [|"--smart-case", "--json", "--", query, workingDirectory|],
    callback,
    completedCallback,
  );
};

let make = path => {search: search(path), findInFiles: findInFiles(path)};
