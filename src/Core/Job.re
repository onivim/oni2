module Time = Revery.Time;

type mapFn('p, 'c) = ('p, 'c) => (bool, 'p, 'c);
type doWork('p, 'c) = mapFn('p, 'c);
type progressReporter('p, 'c) = ('p, 'c) => float;

type workPrinter('a) = 'a => string;

let defaultProgressReporter = (_, _) => 0.;

type t('p, 'c) = {
  f: doWork('p, 'c),
  isComplete: bool,
  pendingWork: 'p,
  completedWork: 'c,
  budget: Time.t,
  name: string,
  progressReporter: progressReporter('p, 'c),
  pendingWorkPrinter: workPrinter('p),
  completedWorkPrinter: workPrinter('c),
};

let defaultBudget = Time.Milliseconds(1.);

let isComplete = (v: t('p, 'c)) => v.isComplete;

let noopPrinter = _ => " (no printer) ";

let getCompletedWork = (v: t('p, 'c)) => v.completedWork;

let getPendingWork = (v: t('p, 'c)) => v.pendingWork;

let getProgress = (v: t('p, 'c)) =>
  if (v.isComplete) {
    1.0;
  } else {
    v.progressReporter(v.pendingWork, v.completedWork);
  };

let create =
    (
      ~f: doWork('p, 'c),
      ~initialCompletedWork: 'c,
      ~name="anonymous",
      ~budget=defaultBudget,
      ~progressReporter=defaultProgressReporter,
      ~pendingWorkPrinter=noopPrinter,
      ~completedWorkPrinter=noopPrinter,
      pendingWork: 'p,
    ) => {
  {
    budget,
    f,
    completedWork: initialCompletedWork,
    pendingWork,
    name,
    isComplete: false,
    progressReporter,
    pendingWorkPrinter,
    completedWorkPrinter,
  };
};

let map = (f: mapFn('p, 'c), v: t('p, 'c)) => {
  let {pendingWork, completedWork, _} = v;
  let (isComplete, pendingWork, completedWork) =
    f(pendingWork, completedWork);
  {...v, isComplete, pendingWork, completedWork};
};

let doWork = (v: t('p, 'c)) => map(v.f, v);

/* Like [map], but do a unit of work after mapping. */
let mapw = (f: mapFn('p, 'c), v: t('p, 'c)) => {
  v |> map(f) |> doWork;
};

let show = (v: t('p, 'c)) => {
  "Name: "
  ++ v.name
  ++ "\n"
  ++ " - Pending work: "
  ++ v.pendingWorkPrinter(v.pendingWork)
  ++ "\n"
  ++ " - Completed work: "
  ++ v.completedWorkPrinter(v.completedWork)
  ++ "\n";
};

let tick: t('p, 'c) => t('p, 'c) =
  (v: t('p, 'c)) => {
    let budget = Time.to_float_seconds(v.budget);
    let startTime = Unix.gettimeofday();
    let current = ref(v);
    let iterations = ref(0);

    Log.debug("[Job] Starting " ++ v.name);
    while (Unix.gettimeofday() -. startTime < budget && !current^.isComplete) {
      current := doWork(current^);
      incr(iterations);
    };

    let endTime = Unix.gettimeofday();

    Log.info(
      "[Job] "
      ++ v.name
      ++ " ran "
      ++ string_of_int(iterations^)
      ++ " iterations for "
      ++ string_of_float(endTime -. startTime)
      ++ "s",
    );

    if (Log.isDebugLoggingEnabled()) {
      Log.debug("[Job] Detailed report: " ++ show(v));
    };

    current^;
  };
