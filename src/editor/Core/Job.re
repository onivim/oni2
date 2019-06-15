open Revery;

type doWork('p, 'c) = ('p, 'c) => (bool, 'p, 'c);
type mapFn('p, 'c) = ('p, 'c) => ('p, 'c);

type t('p, 'c) = {
  f: doWork('p, 'c),
  isComplete: bool,
  pendingWork: 'p,
  completedWork: 'c,
  budget: Time.t,
};

let defaultBudget = Time.Milliseconds(1.);

let isComplete = (v: t('p, 'c)) => v.isComplete;

let create =
    (
      ~budget=defaultBudget,
      ~f: doWork('p, 'c),
      ~initialCompletedWork: 'c,
      pendingWork: 'p,
    ) => {
  {
    budget,
    f,
    completedWork: initialCompletedWork,
    pendingWork,
    isComplete: false,
  };
};

let map = (~f: mapFn('p, 'c), v: t('p, 'c)) => {
  let {pendingWork, completedWork, _} = v;
  let (pendingWork, completedWork) = f(pendingWork, completedWork);
  {...v, pendingWork, completedWork};
};

let doWork = (v: t('p, 'c)) => {
  let (isComplete, pendingWork, completedWork) =
    v.f(v.pendingWork, v.completedWork);
  {...v, isComplete, pendingWork, completedWork};
};

let tick: t('p, 'c) => t('p, 'c) =
  (v: t('p, 'c)) => {
    let budget = Time.to_float_seconds(v.budget);
    let startTime = Time.getTime() |> Time.to_float_seconds;
    let current = ref(v);

    while (Time.to_float_seconds(Time.getTime())
           -. startTime < budget
           && !current^.isComplete) {
      current := doWork(v);
    };

    current^;
  };
