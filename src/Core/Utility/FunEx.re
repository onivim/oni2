let tap = (f, x) => {
  f(x);
  x;
};

type throttleState('a) =
  | NothingWaiting
  | Collecting
  | Throttling({latest: 'a});

let throttle =
    (~leading=true, ~trailing=true, ~timeout=Revery.Tick.timeout, ~time, f) => {
  let throttleState = ref(NothingWaiting);

  let finish = () => {
    switch (throttleState^) {
    | NothingWaiting => () // This shouldn't really happen..
    | Collecting => () // We queued up, but there weren't any further calls - nothing to do
    | Throttling({latest}) =>
      if (trailing) {
        f(latest);
      }
    };

    throttleState := NothingWaiting;
  };

  v => {
    switch (throttleState^) {
    // Nothing is waiting... execute immediately, start timer, collect
    | NothingWaiting =>
      if (leading) {
        f(v);
      };
      let _: unit => unit = timeout(finish, time);
      throttleState := Collecting;
    | Collecting
    | Throttling(_) => throttleState := Throttling({latest: v})
    };
  };
};
