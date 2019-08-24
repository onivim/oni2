open Revery;

type mapFn('p, 'c) = ('p, 'c) => (bool, 'p, 'c);
type doWork('p, 'c) = mapFn('p, 'c);

type workPrinter('a) = 'a => string;

type t('p, 'c) = {
  f: doWork('p, 'c),
  isComplete: bool,
  pendingWork: 'p,
  completedWork: 'c,
  budget: Time.t,
  name: string,
  pendingWorkPrinter: workPrinter('p),
  completedWorkPrinter: workPrinter('c),
};

let defaultBudget = Time.Milliseconds(1.);

let isComplete = (v: t('p, 'c)) => v.isComplete;

let noopPrinter = _ => " (no printer) ";

let getCompletedWork = (v: t('p, 'c)) => v.completedWork;

let getPendingWork = (v: t('p, 'c)) => v.pendingWork;

let create =
    (
      ~f: doWork('p, 'c),
      ~initialCompletedWork: 'c,
      ~name="anonymous",
      ~budget=defaultBudget,
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
    let startTime = Time.getTime() |> Time.to_float_seconds;
    let current = ref(v);

    Log.debug("[Job] Starting " ++ v.name);
    while (Time.to_float_seconds(Time.getTime())
           -. startTime < budget
           && !current^.isComplete) {
      current := doWork(v);
    };

    let endTime = Time.to_float_seconds(Time.getTime());

    Log.info("[Job] " ++ v.name ++ " ran for " ++ string_of_float(endTime -. startTime));

    if (Log.isDebugLoggingEnabled()) {
      Log.debug("[Job] Detailed report: " ++ show(v));
    };

    current^;
  };
