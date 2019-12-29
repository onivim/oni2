open Rench;

module Time = Revery_Core.Time;
module List = Utility.List;
module ListEx = Utility.ListEx;
module Queue = Utility.Queue;

module Match = {
  module Log = (val Log.withNamespace("Oni2.Ripgrep.Match"));

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
        Log.error("Error decoding JSON: " ++ message);
        None;
      | Yojson.Json_error(message) =>
        Log.error("Error parsing JSON: " ++ message);
        None;
      }
    );
  };
};

module Log = (val Log.withNamespace("Oni2.Ripgrep"));

type t = {
  search:
    (
      ~filesExclude: list(string),
      ~directory: string,
      ~onUpdate: list(string) => unit,
      ~onComplete: unit => unit
    ) =>
    dispose,
  findInFiles:
    (
      ~directory: string,
      ~query: string,
      ~onUpdate: list(Match.t) => unit,
      ~onComplete: unit => unit
    ) =>
    dispose,
}

and dispose = unit => unit;

/**
 RipgrepProcessingJob is the logic for processing a [Bytes.t]
 and sending it back to whatever is listening to Ripgrep.

 Each iteration of work, it will split up a [Bytes.t] (a chunk of output),
 get a list of files, and send that to the callback.
*/
module RipgrepProcessingJob = {
  type pendingWork = {
    onUpdate: list(string) => unit,
    onComplete: unit => unit,
    queue: Queue.t(Bytes.t),
  };

  let pendingWorkPrinter = pending =>
    Printf.sprintf("Byte chunks left: %n", Queue.length(pending.queue));

  type t = Job.t(pendingWork, unit);

  let doWork = (pending, completed) => {
    let queue =
      switch (Queue.pop(pending.queue)) {
      | (None, queue) => queue
      | (Some(bytes), queue) =>
        let items =
          bytes
          |> Bytes.to_string
          |> String.trim
          |> String.split_on_char('\n');
        pending.onUpdate(items);
        queue;
      };

    if (Queue.isEmpty(pending.queue)) {
      pending.onComplete();
    };

    (Queue.isEmpty(pending.queue), {...pending, queue}, completed);
  };

  let create = (~onUpdate, ~onComplete) => {
    Job.create(
      ~f=doWork,
      ~initialCompletedWork=(),
      ~name="RipgrepProcessingJob",
      ~pendingWorkPrinter,
      ~budget=Time.ms(2),
      {onUpdate, onComplete, queue: Queue.empty},
    );
  };

  let queueWork = (bytes: Bytes.t, currentJob: t) => {
    Job.map(
      (pending, completed) => {
        let queue = Queue.push(bytes, pending.queue);
        (false, {...pending, queue}, completed);
      },
      currentJob,
    );
  };
};

let process = (rgPath, args, onUpdate, onComplete) => {
  let argsStr = String.concat("|", Array.to_list(args));
  Log.infof(m => m("Starting process: %s with args: |%s|", rgPath, argsStr));

  // Mutex to
  let jobMutex = Mutex.create();
  let job = ref(RipgrepProcessingJob.create(~onUpdate, ~onComplete));

  let disposeTick = ref(None);

  Revery.App.runOnMainThread(() =>
    disposeTick :=
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

  let childProcess = ChildProcess.spawn(rgPath, args);

  let disposeOnData =
    Event.subscribe(
      childProcess.stdout.onData,
      value => {
        Mutex.lock(jobMutex);
        job := RipgrepProcessingJob.queueWork(value, job^);
        Mutex.unlock(jobMutex);
      },
    );

  let disposeOnClose =
    Event.subscribe(childProcess.onClose, exitCode => {
      Log.infof(m => m("Process completed - exit code: %n", exitCode))
    });

  let dispose = () => {
    Log.info("Session complete.");
    disposeOnData();
    disposeOnClose();
    switch (disposeTick^) {
    | Some(dispose) => dispose()
    | None => ()
    };
    childProcess.kill(Sys.sigkill);
  };

  dispose;
};

/**
   Search through files of the directory passed in and sort the results in
   order of the last time they were accessed, alternative sort order includes
   path, modified, created
 */
let search =
    (~executablePath, ~filesExclude, ~directory, ~onUpdate, ~onComplete) => {
  let dedup = {
    let seen = Hashtbl.create(1000);

    List.filter(str => {
      switch (Hashtbl.find_opt(seen, str)) {
      | Some(_) => false
      | None =>
        Hashtbl.add(seen, str, true);
        true;
      }
    });
  };

  let globs =
    filesExclude
    |> List.map(x => "!" ++ x)
    |> List.map(x => ["-g", x])
    |> List.concat;

  let args =
    globs
    @ ["--smart-case", "--hidden", "--files", "--", directory]
    |> Array.of_list;

  process(
    executablePath,
    args,
    items => items |> dedup |> onUpdate,
    onComplete,
  );
};

let findInFiles =
    (~executablePath, ~directory, ~query, ~onUpdate, ~onComplete) => {
  process(
    executablePath,
    [|"--smart-case", "--hidden", "--json", "--", query, directory|],
    items => {
      items
      |> List.filter_map(Match.fromJsonString)
      |> ListEx.safeConcat
      |> onUpdate
    },
    onComplete,
  );
};

let make = (~executablePath) => {
  search: search(~executablePath),
  findInFiles: findInFiles(~executablePath),
};
