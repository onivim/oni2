module Log = (val Timber.Log.withNamespace("Exthost.Waiter"));

let defaultFail = (~name, msg) => failwith(name ++ ":" ++ msg);

let wait = (~onFail=defaultFail, ~timeout=10.0, ~name="TODO", condition) => {
  let start = Unix.gettimeofday();
  let delta = () => Unix.gettimeofday() -. start;

  Log.infof(m => m("Starting waiter '%s' at %f", name, start));

  while (!condition() && delta() < timeout) {
    for (_i in 0 to 100) {
      ignore(Luv.Loop.run(~mode=`NOWAIT, ()): bool);
    };
    Unix.sleepf(0.1);
  };

  if (!condition()) {
    Log.errorf(m => m("Waiter failed '%s' after %f seconds", name, delta()));
    prerr_endline("=== CONDITION FAILED: " ++ name);

    // Make test failures a bit easier to investigate, otherwise the amount of logging
    // from nearby successful tests will just bury the failure.
    onFail(~name, "Waiter failed");
  } else {
    Log.infof(m =>
      m("Waiter completed '%s' after %f seconds", name, delta())
    );
  };
};

let waitForCollection = (~name, item) => {
  let collected = ref(false);

  let checkCollected = () => {
    Gc.full_major();
    collected^ == true;
  };

  Gc.finalise_last(() => collected := true, item);
  wait(~name="Waiting for GC to collect: " ++ name, checkCollected);
};
