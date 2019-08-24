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

module RipgrepThread = {

  type pendingWork = {
    callback: (list(string)) => unit,
    bytes: list(Bytes.t),
  };

  type t = {
    job: ref(Job.t(pendingWork, unit)),
    isRunning: ref(bool)
  };

  let doWork = (pendingWork, c) => {

    let newBytes = switch (pendingWork.bytes) {
    | [] => [] 
    | [hd, ...tail] => 
              prerr_endline ("rgt - 1");
              let items = Bytes.to_string(hd)
              prerr_endline ("rgt - 2");
                let items = String.trim(items);
              prerr_endline ("rgt - 3");
                let items = String.split_on_char('\n', items);
              prerr_endline ("rgt - 4");
                let items = items |> List.filter(isNotDirectory);
              prerr_endline ("rgt - 5");
              pendingWork.callback(items);
              prerr_endline ("rgt - 5");
              tail;
    }

    let isDone = switch(newBytes) {
    | [] => true
    | _ => false;
    };
    
    (isDone, {...pendingWork, bytes: newBytes }, c);
  };

  let pendingWorkPrinter = (p: pendingWork) => {
    "Byte chunks left: " ++ string_of_int(List.length(p.bytes));
  };

  let start = (callback) => {

    let j = Job.create(~f=doWork, 
               ~initialCompletedWork=(),
               ~name="RipgrepProcessorJob",
               ~pendingWorkPrinter,
               {
                callback,
                bytes: []
               });
              
    
    
    let isRunning = ref(true);
    let job = ref(j);
  
    let _ = Thread.create(() => {
      Log.info("[RipgrepThread] Starting...");
      while (isRunning^ || !Job.isComplete(job^)) {
        job := Job.tick(job^);
        if (Log.isDebugLoggingEnabled()) {
          Log.debug("[RipgrepThread] Work: " ++ Job.show(job^));
        }
        Unix.sleepf(0.01);
      }
      Log.info("[RipgrepThread] Finished...");
    }, ());

      let ret: t = {
        isRunning,
        job,
      }

      ret;
    };

  let stop = (v: t) => {
    v.isRunning := false;
  };

  let queueWork = (v: t, bytes: Bytes.t) => {
    let currentJob = v.job^; 
    v.job := Job.map((p, c) => {
      let newP: pendingWork = {
        ...p,
        bytes: [bytes, ...p.bytes],
      };
      (false, newP, c)
    }, currentJob);
  };
};

let process = (workingDirectory, rgPath, args, callback, completedCallback) => {
  incr(_ripGrepRunCount);
  let argsStr = String.concat("|", Array.to_list(args));
  Log.info("[Ripgrep] Starting process: " ++ rgPath ++ " with args: |" ++ argsStr ++ "|");
  let cp = ChildProcess.spawn(~cwd=Some(workingDirectory), rgPath, args);

  let processingThread = RipgrepThread.start((items) => Revery.App.runOnMainThread(() => callback(items)));

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
        dispose3();
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
