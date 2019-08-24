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

/**
 RipgrepThreadJob is the actual logic for processing a [Bytes.t]
 and sending it to the main thread. Each iteration of work,
 it will split up a [Bytes.t] (a chunk of ripgrep output) and
 get a list of files, and send that to the callback
*/
module RipgrepThreadJob = {
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

module RipgrepThread = {

  type t = {
    mutex: Mutex.t,
    job: ref(RipgrepThreadJob.t),
    isRunning: ref(bool),
    rgActive: ref(bool),
  };

  let start = callback => {
    let j = RipgrepThreadJob.create(~callback, ());

    let rgActive = ref(true);
    let isRunning = ref(true);
    let job = ref(j);
    let mutex = Mutex.create();

    let _ =
      Thread.create(
        () => {
          Log.info("[RipgrepThread] Starting...");
          while (isRunning^ && (rgActive^ || !Job.isComplete(job^))) {
            Mutex.lock(mutex);
            job := Job.tick(job^);
            Mutex.unlock(mutex);
            if (Log.isDebugLoggingEnabled()) {
              Log.debug("[RipgrepThread] Work: " ++ Job.show(job^));
            };
            Unix.sleepf(0.001);
          };
          Log.info("[RipgrepThread] Finished...");
        },
        (),
      );

    let ret: t = {mutex, rgActive, isRunning, job};

    ret;
  };

  let stop = (v: t) => {
    Mutex.lock(v.mutex);
    v.isRunning := false;
    Mutex.unlock(v.mutex);
  };

  let notifyRipgrepFinished = (v: t) => {
    Mutex.lock(v.mutex);
    v.rgActive := false;
    Mutex.unlock(v.mutex);
  };

let queueWork = (v: t, bytes: Bytes.t) => {
    Mutex.lock(v.mutex);
    let currentJob = v.job^;
    v.job := RipgrepThreadJob.queueWork(bytes, currentJob);
    Mutex.unlock(v.mutex);
  };
};

let process = (workingDirectory, rgPath, args, callback, completedCallback) => {
  incr(_ripGrepRunCount);
  let argsStr = String.concat("|", Array.to_list(args));
  Log.info(
    "[Ripgrep] Starting process: "
    ++ rgPath
    ++ " with args: |"
    ++ argsStr
    ++ "|",
  );

  /**
   For processing Ripgrep, we do a few things:
   1) Run the actual ripgrep process
   2) The process pipes its data into a [RipgrepThread] which takes the raw [Bytes.t] output and translates it into lines
   3) The lines are sent to the main thread via a callback to be dispatched against the store
   */

  let cp = ChildProcess.spawn(~cwd=Some(workingDirectory), rgPath, args);
  let processingThread =
    RipgrepThread.start(items =>
      Revery.App.runOnMainThread(() => callback(items))
    );

  let dispose3 = () => RipgrepThread.stop(processingThread);

  let dispose1 =
    Event.subscribe(
      cp.stdout.onData,
      value => {
        RipgrepThread.queueWork(processingThread, value);
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
        RipgrepThread.notifyRipgrepFinished(processingThread);
        completedCallback();
      },
    );

  () => {
    dispose1();
    dispose2();
    dispose3();
    cp.kill(Sys.sigkill);
  };
};

/**
   Search through files of the directory passed in and sort the results in
   order of the last time they were accessed, alternative sort order includes
   path, modified, created
 */
let search = (path, search, workingDirectory, callback, completedCallback) => {
  // TODO: We ignore the search parameter for now because, if we specify a glob filter,
  // it will override the .gitignore globs - potentially searching in ignored folders.
  ignore(search);
  process(
    workingDirectory,
    path,
    [|"--smart-case", "--files", "--block-buffered"|],
    callback,
    completedCallback,
  );
};

let make = path => {search: search(path)};
