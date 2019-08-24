open Rench;

type disposeFunction = unit => unit;

/* Internal counters used for tracking */
let _ripGrepRunCount = ref(0);
let _ripGrepCompletedCount = ref(0);

let getRunCount = () => _ripGrepRunCount^;
let getCompletedCount = () => _ripGrepCompletedCount^;

type searchFunction('a) = (string => ('a), string, list('a) => unit, unit =>  unit) => disposeFunction;

type t('a) = {
  search: searchFunction('a),
};

/**
 RipgrepProcessingJob is the logic for processing a [Bytes.t]
 and sending it back to whatever is listening to Ripgrep.

 Each iteration of work, it will split up a [Bytes.t] (a chunk of output),
 get a list of files, and send that to the callback.
*/
module RipgrepProcessingJob = {

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

let process = (rgPath, mapItems, args, callback, completedCallback) => {
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
  let job = ref(RipgrepProcessingJob.create(~mapItems, ~callback, ()));
  let rgDone = ref(false);
  let completed = ref(false);


  let dispose3 = ref(None);
  
  
  Revery.App.runOnMainThread(() => {
    dispose3 := Some(Revery.Tick.interval(_ => {
      if (!Job.isComplete(job^)) {
        Mutex.lock(jobMutex);
        job := Job.tick(job^);
        Mutex.unlock(jobMutex);
      } else if (rgDone^ && !completed^) {
        completed := true;
        completedCallback();
      }
    }, Milliseconds(0.)));
  });
  
  let cp = ChildProcess.spawn(rgPath, args);

  let dispose1 =
    Event.subscribe(cp.stdout.onData, value => {
      Mutex.lock(jobMutex);
      // print_endline ("Queuing work");
      job := RipgrepProcessingJob.queueWork(value, job^);
      Mutex.unlock(jobMutex);
    });

  let dispose2 =
    Event.subscribe(
      cp.onClose,
      exitCode => {
        incr(_ripGrepCompletedCount);
        Log.info(
          "Ripgrep completed - exit code: " ++ string_of_int(exitCode),
        );
        rgDone := true;
      },
    );

  () => {
    print_endline("!!DISPOSING!!");
    dispose1();
    dispose2();
    switch(dispose3^) {
    |Some(v) => v();
    | None => ();
    }
    cp.kill(Sys.sigkill);
  };
};

/**
   Search through files of the directory passed in and sort the results in
   order of the last time they were accessed, alternative sort order includes
   path, modified, created
 */
let search = (path, mapItems, workingDirectory, callback, completedCallback) => {
  process(
    path,
    mapItems,
    [|
      "--smart-case",
      "--files",
      "--",
      workingDirectory,
    |],
    callback,
    completedCallback,
  );
};

let make = path => {search: search(path)};
