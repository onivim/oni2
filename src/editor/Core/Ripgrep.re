open Rench;

type disposeFunction = unit => unit;

type searchFunction('a) = (string => ('a), string, string, list('a) => unit, unit =>  unit) => disposeFunction;

type t('a) = {
  search: searchFunction('a),
};

/* Internal counters used for tracking */
let _ripGrepRunCount = ref(0);
let _ripGrepCompletedCount = ref(0);

let getRunCount = () => _ripGrepRunCount^;
let getCompletedCount = () => _ripGrepCompletedCount^;

/**
 RipgrepThreadJob is the actual logic for processing a [Bytes.t]
 and sending it to the main thread. Each iteration of work,
 it will split up a [Bytes.t] (a chunk of ripgrep output) and
 get a list of files, and send that to the callback
*/
module RipgrepThreadJob = {
  type pendingWork('a) = {
    duplicateHash: Hashtbl.t(string, bool),
    itemMapping: string => 'a,
    callback: list('a) => unit,
    bytes: list(Bytes.t),
  };

  let pendingWorkPrinter = (p: pendingWork('a)) => {
    "Byte chunks left: " ++ string_of_int(List.length(p.bytes));
  };

  type t('a) = Job.t(pendingWork('a), unit);

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
          |> List.filter(dedup(pendingWork.duplicateHash))
          |> List.map(pendingWork.itemMapping);
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

  let create = (~mapItems, ~callback, ()) => {
    let duplicateHash = Hashtbl.create(1000);
    Job.create(
      ~f=doWork,
      ~initialCompletedWork=(),
      ~name="RipgrepProcessorJob",
      ~pendingWorkPrinter,
      {callback, bytes: [], duplicateHash, itemMapping:mapItems},
    );
  };

  let queueWork = (bytes: Bytes.t, currentJob: t('a)) => {
    Job.map(
      (p, c) => {
        let newP: pendingWork('a) = {...p, bytes: [bytes, ...p.bytes]};
        (false, newP, c);
      },
      currentJob,
    );
  };
};

module RipgrepThread = {
  type t('a) = {
    mutex: Mutex.t,
    job: ref(RipgrepThreadJob.t('a)),
    isRunning: ref(bool),
    rgActive: ref(bool),
    signal: Condition.t,
  };

  let start = (mapItems, callback, onCompleteCallback) => {
    let j = RipgrepThreadJob.create(~mapItems, ~callback, ());

    let rgActive = ref(true);
    let isRunning = ref(true);
    let job = ref(j);
    let mutex = Mutex.create();
    let signal = Condition.create();

    let _ =
      Thread.create(
        () => {
          Log.info("[RipgrepThread] Starting...");
          while (isRunning^ && (rgActive^ || !Job.isComplete(job^))) {

            if (Job.isComplete(job^)) {
              Log.debug("[RipgrepThread] Waiting on work...");
              let _ = Condition.wait(signal);
              Log.debug("[RipgrepThread] Got work!");
            }

            Mutex.lock(mutex);
            job := Job.tick(job^);
            Mutex.unlock(mutex);
            if (Log.isDebugLoggingEnabled()) {
              Log.debug("[RipgrepThread] Work: " ++ Job.show(job^));
            };
            Unix.sleepf(0.1);
          };
          Log.info("[RipgrepThread] Finished...");
          onCompleteCallback();
        },
        (),
      );

    let ret: t('a) = {mutex, rgActive, isRunning, job, signal};

    ret;
  };

  let stop = (v: t('a)) => {
    Mutex.lock(v.mutex);
    v.isRunning := false;
    let () = Condition.signal(v.signal);
    Mutex.unlock(v.mutex);
  };

  let notifyRipgrepFinished = (v: t('a)) => {
    Mutex.lock(v.mutex);
    v.rgActive := false;
    Mutex.unlock(v.mutex);
  };

  let queueWork = (v: t('a), bytes: Bytes.t) => {
    Mutex.lock(v.mutex);
    let currentJob = v.job^;
    v.job := RipgrepThreadJob.queueWork(bytes, currentJob);
    let () = Condition.signal(v.signal);
    Mutex.unlock(v.mutex);
  };
};

let process = (mapItems, workingDirectory, rgPath, args, callback, completedCallback) => {
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

  let processingThread =
    RipgrepThread.start(mapItems,
      items => Revery.App.runOnMainThread(() => {
      // Amortize the cost of GC across multiple frames
      let _ = Gc.major_slice(0);
      callback(items);
      }),
      // Previously, we sent the completed callback as soon as the Ripgrep process is done,
      // but that isn't accurate anymore with the RipgrepThreadJob - we're only done once
      // that thread is done processing results!
      () =>
        Revery.App.runOnMainThread(() => {
          Log.info("[Ripgrep] Processing thread sending completed callback.");
          completedCallback();
        }),
    );

  let dispose3 = () => RipgrepThread.stop(processingThread);

  let cp = ChildProcess.spawn(~cwd=Some(workingDirectory), rgPath, args);

  let dispose1 =
    Event.subscribe(
      cp.stdout.onData,
      value => {
        Log.debug(
          "[Ripgrep] Queuing work from stdout: "
          ++ string_of_int(Bytes.length(value)),
        );
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
let search = (path, mapItems, search, workingDirectory, callback, completedCallback) => {
  // TODO: We ignore the search parameter for now because, if we specify a glob filter,
  // it will override the .gitignore globs - potentially searching in ignored folders.
  ignore(search);
  process(
    mapItems,
    workingDirectory,
    path,
    [|"--smart-case", "--files", "--block-buffered"|],
    callback,
    completedCallback,
  );
};

let make = (path) => {
  search: search(path),
};
