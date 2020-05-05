let tap = (f, x) => {
  f(x);
  x;
};

type debounceState('a) =
  | NothingWaiting
  | Collecting
  | Debouncing({latest: 'a});

let debounce1 = (~timeout=Revery.Tick.timeout, ~time, f) => {
  let debounceState = ref(NothingWaiting);

  let finish = () => {
    switch (debounceState^) {
    | NothingWaiting => () // This shouldn't really happen..
    | Collecting => () // We queued up, but there weren't any further calls - nothing to do
    | Debouncing({latest}) => f(latest)
    };

    debounceState := NothingWaiting;
  };

  v => {
    switch (debounceState^) {
    // Nothing is waiting... execute immediately, start timer, collect
    | NothingWaiting =>
      f(v);
      let _: unit => unit = timeout(finish, time);
      debounceState := Collecting;
    | Collecting
    | Debouncing(_) => debounceState := Debouncing({latest: v})
    };
  };
};
