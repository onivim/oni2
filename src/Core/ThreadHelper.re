/*
 * ThreadHelper.re
 *
 * Simple console logger
 */

let activeThreadMutex = Mutex.create();
let activeThreads: ref(list(Thread.t)) = ref([]);

module Log = (val Log.withNamespace("Oni2.ThreadHelper"));

let create = (~name="Anonymous", f, args) => {
  let thread =
    Thread.create(
      threadArgs => {
        let self = Thread.self();
        let id = Thread.id(self);

        Mutex.lock(activeThreadMutex);
        activeThreads := [self, ...activeThreads^];
        Mutex.unlock(activeThreadMutex);

        Log.infof(m => m("Starting thread: %d (%s)", id, name));
        try(f(threadArgs)) {
        | ex =>
          Log.errorf(m =>
            m(
              "Thread %d (%s) failed with: %s",
              id,
              name,
              Printexc.to_string(ex),
            )
          )
        };

        Log.infof(m => m("Closing thread: %d (%s)", id, name));
        Mutex.lock(activeThreadMutex);
        activeThreads := List.filter(t => t !== self, activeThreads^);
        Mutex.unlock(activeThreadMutex);
      },
      args,
    );

  thread;
};

let showRunningThreads = () => {
  Mutex.lock(activeThreadMutex);
  let output =
    activeThreads^
    |> List.map(Thread.id)
    |> List.map(s => "Thread: " ++ string_of_int(s))
    |> String.concat("\n");

  Mutex.unlock(activeThreadMutex);
  output;
};

let killAll = () => {
  Mutex.lock(activeThreadMutex);
  activeThreads^ |> List.iter(Thread.kill);
  Mutex.unlock(activeThreadMutex);
};
