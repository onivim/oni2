open Revery;

type doWork('p, 'c) = ('p, 'c) => ((bool, 'p, 'c));

type t('p, 'c) = {
    f: doWork('p, 'c),
    isComplete: bool,
    pendingWork: 'p,
    completedWork: 'c,
    budget: Time.t,
};

let create = (~budget=Milliseconds(1.), ~f: doWork('p, 'c), ~initialCompletedWork: 'c, pendingWork: 'p) => {
    { budget, f, completedWork: initialCompletedWork, pendingWork, isComplete: false }
};

let doWork = (v: t('p, 'c)) => {
    let (isCompleted, pendingWork, completedWork) = v.doWork(v.pendingWork, v.completedWork);
    { ...v, isCompleted, pendingWork, completedWork }
}

let tick: t('p, 'c) => t('p, 'c) = (v: t('p, 'c)) => {
    let budget = Time.toSeconds(v.budget);
    let startTime = Time.getTime() |> Time.toSeconds;
    let current = ref(v);

    while (Time.toSeconds(Time.getTime()) -. startTime < budget && !current^.isCompleted) {
        current := doWork(v);
    };

    current^;
}
