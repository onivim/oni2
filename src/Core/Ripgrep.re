open Kernel;
open Utility;

module Time = Revery_Core.Time;

module Match = {
  module Log = (val Log.withNamespace("Oni2.Core.Ripgrep.Match"));

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
        Log.warn("Error decoding JSON: " ++ message);
        None;
      | Yojson.Json_error(message) =>
        Log.warn("Error parsing JSON: " ++ message);
        None;
      }
    );
  };
};

module Log = (val Log.withNamespace("Oni2.Core.Ripgrep"));

type t = {
  search:
    (
      ~followSymlinks: bool,
      ~useIgnoreFiles: bool,
      ~filesExclude: list(string),
      ~directory: string,
      ~onUpdate: list(string) => unit,
      ~onComplete: unit => unit,
      ~onError: string => unit
    ) =>
    dispose,
  findInFiles:
    (
      ~followSymlinks: bool,
      ~useIgnoreFiles: bool,
      ~searchExclude: list(string),
      ~searchInclude: list(string),
      ~directory: string,
      ~query: string,
      ~onUpdate: list(Match.t) => unit,
      ~onComplete: unit => unit,
      ~onError: string => unit,
      ~enableRegex: bool=?,
      ~caseSensitive: bool=?,
      unit
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

    (Queue.isEmpty(pending.queue), {...pending, queue}, completed);
  };

  let create = (~onUpdate) => {
    Job.create(
      ~f=doWork,
      ~initialCompletedWork=(),
      ~name="RipgrepProcessingJob",
      ~pendingWorkPrinter,
      ~budget=Time.ms(2),
      {onUpdate, queue: Queue.empty},
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

let process = (rgPath, args, onUpdate, onComplete, onError) => {
  let on_exit = (_, ~exit_status: int64, ~term_signal as _) => {
    Log.debugf(m =>
      m("Process completed - exit code: %n", exit_status |> Int64.to_int)
    );
  };

  Log.debugf(m =>
    m(
      "Starting process: %s with args: |%s|",
      rgPath,
      args |> String.concat(" "),
    )
  );

  let processResult =
    Luv.Pipe.init()
    |> ResultEx.flatMap(pipe => {
         LuvEx.Process.spawn(
           ~on_exit,
           ~redirect=[
             Luv.Process.to_parent_pipe(
               ~fd=Luv.Process.stdout,
               ~parent_pipe=pipe,
               (),
             ),
           ],
           rgPath,
           [rgPath, ...args],
         )
         |> Result.map(process => (process, pipe))
       });

  switch (processResult) {
  | Error(err) =>
    let errMsg = err |> Luv.Error.strerror;
    Log.error(errMsg);
    onError(errMsg);
    (() => ());
  | Ok((process, pipe)) =>
    let job = ref(RipgrepProcessingJob.create(~onUpdate));
    let isRipgrepProcessDone = ref(false);

    let disposeTick = ref(None);
    let disposeAll = () => {
      Log.info("disposeAll");
      disposeTick^ |> Option.iter(f => f());
      let _: result(unit, Luv.Error.t) = Luv.Process.kill(process, 2);
      isRipgrepProcessDone := true;
      ();
    };

    let allocator = Utility.LuvEx.allocator("Ripgrep");
    Luv.Stream.read_start(
      ~allocate=allocator,
      pipe,
      fun
      | Error(`EOF) => {
          Luv.Handle.close(pipe, ignore);
          isRipgrepProcessDone := true;
        }
      | Error(msg) => {
          disposeAll();
          let errMsg = msg |> Luv.Error.strerror;

          onError(errMsg);
        }
      | Ok(buffer) => {
          let bytes = Luv.Buffer.to_bytes(buffer);
          job := RipgrepProcessingJob.queueWork(bytes, job^);
        },
    );

    disposeTick :=
      Some(
        Revery.Tick.interval(
          ~name="Ripgrep - Processing Ticker",
          _ =>
            if (!Job.isComplete(job^)) {
              job := Job.tick(job^);
            } else if (isRipgrepProcessDone^) {
              onComplete();
              disposeAll();
            },
          Time.zero,
        ),
      );

    disposeAll;
  };
};

/**
   Search through files of the directory passed in and sort the results in
   order of the last time they were accessed, alternative sort order includes
   path, modified, created
 */
let search =
    (
      ~executablePath,
      ~followSymlinks,
      ~useIgnoreFiles,
      ~filesExclude,
      ~directory,
      ~onUpdate,
      ~onComplete,
      ~onError,
    ) => {
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

  let followArgs = followSymlinks ? ["--follow"] : [];

  let ignoreArgs = useIgnoreFiles ? [] : ["--no-ignore"];

  let args =
    globs
    @ followArgs
    @ ignoreArgs
    @ ["--smart-case", "--hidden", "--files", "--", directory];

  process(
    executablePath,
    args,
    items => items |> dedup |> onUpdate,
    onComplete,
    onError,
  );
};

let findInFiles =
    (
      ~executablePath,
      ~followSymlinks,
      ~useIgnoreFiles,
      ~searchExclude,
      ~searchInclude,
      ~directory,
      ~query,
      ~onUpdate,
      ~onComplete,
      ~onError,
      ~enableRegex=false,
      ~caseSensitive=false,
      (),
    ) => {
  let excludeArgs =
    searchExclude
    |> List.filter(str => !StringEx.isEmpty(str))
    |> List.concat_map(x => ["-g", "!" ++ x]);
  let includeArgs =
    searchInclude
    |> List.filter(str => !StringEx.isEmpty(str))
    |> List.concat_map(x => ["-g", x]);

  let followArgs = followSymlinks ? ["--follow"] : [];

  let ignoreArgs = useIgnoreFiles ? [] : ["--no-ignore"];

  let args =
    excludeArgs
    @ includeArgs
    @ ignoreArgs
    @ (enableRegex ? [] : ["--fixed-strings"])
    @ followArgs
    @ (caseSensitive ? ["--case-sensitive"] : ["--ignore-case"])
    @ ["--hidden", "--json", "--", query, directory];
  process(
    executablePath,
    args,
    items => {
      items
      |> List.filter_map(Match.fromJsonString)
      |> ListEx.safeConcat
      |> onUpdate
    },
    onComplete,
    onError,
  );
};

let make = (~executablePath) => {
  search: search(~executablePath),
  findInFiles: findInFiles(~executablePath),
};
