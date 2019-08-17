open Oni_Model;
open Oni_IntegrationTestLib;

// This test validates that, in the normal case, dispatching 'Tick' actions doesn't update the state
// If the 'Tick' action always updates state, indiscriminately, it means we'll constantly
// be re-rendering and using a lot of unnecessary CPU!
runTest(~name="TickTest", (dispatch, wait, runEffects) => {
  let oldState = ref(None);
  wait(~name="Capture initial state", (state: State.t) =>
    state.mode == Vim.Types.Normal
  );

  // We'll flush a few rounds of effects
  let i = ref(0);
  while (i^ < 5) {
    dispatch(Tick);
    runEffects();

    wait(~name="Validate no state update", (state: State.t) => {
      oldState := Some(state);
      true;
    });
    incr(i);
  };
  // But this next round should be the same as before..
  dispatch(Tick);
  runEffects();

  wait(~name="Validate no state update", (state: State.t) =>
    switch (oldState^) {
    | Some(v) => v === state
    | None => false
    }
  );
});
