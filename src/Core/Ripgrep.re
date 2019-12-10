open Rench;

module Time = Revery_Core.Time;

module List = Utility.List;

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
  search:
    (
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
    callback: list(string) => unit,
    queue: Queue.t(Bytes.t) // WARNING: mutable data structure
  };

  let pendingWorkPrinter = pending =>
    Printf.sprintf("Byte chunks left: %n", Queue.length(pending.queue));

  type t = Job.t(pendingWork, unit);

  let doWork = (pending, completed) => {
    switch (Queue.take(pending.queue)) {
    | exception Queue.Empty => ()
    | bytes =>
      let items =
        bytes |> Bytes.to_string |> String.trim |> String.split_on_char('\n');
      pending.callback(items);
    };

    let isDone = Queue.is_empty(pending.queue);

    (isDone, pending, completed);
  };

  let create = (~callback, ()) => {
    Job.create(
      ~f=doWork,
      ~initialCompletedWork=(),
      ~name="RipgrepProcessingJob",
      ~pendingWorkPrinter,
      ~budget=Time.ms(2),
      {callback, queue: Queue.create()},
    );
  };

  let queueWork = (bytes: Bytes.t, currentJob: t) => {
    Job.map(
      (pending, completed) => {
        Queue.push(bytes, pending.queue);
        (false, pending, completed);
      },
      currentJob,
    );
  };
};

let process = (rgPath, args, callback, completedCallback) => {
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
    Event.subscribe(
      childProcess.onClose,
      exitCode => {
        Log.info(
          "[Ripgrep] Process completed - exit code: "
          ++ string_of_int(exitCode),
        );
        completedCallback();
      },
    );

  let dispose = () => {
    Log.info("Ripgrep session complete.");
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
let search = (~executablePath, ~directory, ~onUpdate, ~onComplete) => {
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

  process(
    executablePath,
    [|"--smart-case", "--files", "--", directory|],
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
      |> List.concat
      |> onUpdate
    },
    onComplete,
  );
};

let make = (~executablePath) => {
  search: search(~executablePath),
  findInFiles: findInFiles(~executablePath),
};
