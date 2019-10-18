module Time = Revery.Time;

type mapFn('p, 'c) = ('p, 'c) => (bool, 'p, 'c);
type doWork('p, 'c) = mapFn('p, 'c);
type progressReporter('p, 'c) = ('p, 'c) => float;

type workPrinter('a) = 'a => string;

let defaultProgressReporter = (_, _) => 0.;

type t('p, 'c) = {
  job: ref(Job.t('p, 'c)),
  thread: ref(option(Thread.t)),
  isStopping: ref(bool),
  mutex: Mutex.t,
};

let _sync = (v: t('p, 'c), f) => {
  switch (v.thread^) {
  // No active thread, no-op
  | None => ()
  // There is an active thread. Signal it to stop and then wait for it.
  | Some(t) => 
    //v.isStopping := true;
    Thread.join(t);
  }

  Mutex.lock(v.mutex);
  let ret = f();
  Mutex.unlock(v.mutex);
  ret;

};

let tick = (v: t('p, 'c)) => {
  switch (v.thread^) {
  // Already an active thread? No-op
  | Some(_) => ()
  | None =>
    v.isStopping := false;
    v.thread := Some(Thread.create(() => {
      while (!(v.isStopping^) && !Job.isComplete(v.job^)) {
        Thread.yield();
        Mutex.lock(v.mutex);
        v.job := Job.doWork(v.job^)
        Mutex.unlock(v.mutex);
        Thread.yield();
      }
      v.thread := None;
    },()));
  }

  v;
};

let getCompletedWork = (v: t('p, 'c)) => {
  _sync(v, () =>
  Job.getCompletedWork(v.job^));
};

let isComplete = (v: t('p, 'c)) => {
  _sync(v, () => Job.isComplete(v.job^));
};

let getPendingWork = (v: t('p, 'c)) => {
  _sync(v, () => {
  Job.getPendingWork(v.job^) });
};

let map = (f: Job.mapFn('p, 'c), v: t('p, 'c)) => {
  _sync(v, () => { 
    let newJob = Job.map(f, v.job^); 
    v.job := newJob;
  });
  v;
};

let create =
    (
      ~f: doWork('p, 'c),
      ~initialCompletedWork: 'c,
      ~name="anonymous",
      ~progressReporter=Job.defaultProgressReporter,
      ~pendingWorkPrinter=Job.noopPrinter,
      ~completedWorkPrinter=Job.noopPrinter,
      pendingWork: 'p,
    ) => {

  let innerJob = Job.create(
    ~f,
    ~initialCompletedWork,
    ~name,
    ~budget=Time.Seconds(0.),
    ~progressReporter,
    ~pendingWorkPrinter,
    ~completedWorkPrinter,
    pendingWork);

  {
    job: ref(innerJob),
    thread: ref(None),
    isStopping: ref(false),
    mutex: Mutex.create(),
  }
};

