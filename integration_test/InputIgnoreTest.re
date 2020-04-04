open Oni_Core;
open Oni_Model;
open Oni_IntegrationTestLib;

module Log = (val Log.withNamespace("IntegrationTest.inputIgnore"));

// This test validates that certain keystrokes are ignored by our Vim layer
runTest(~name="InputIgnore test", (dispatch, wait, runEffects) => {
  wait(~name="Initial mode is normal", (state: State.t) =>
    state.vimMode == Vim.Types.Normal
  );

  dispatch(KeyboardInput("i"));

  wait(~name="Mode switches to insert", (state: State.t) =>
    state.vimMode == Vim.Types.Insert
  );

  dispatch(KeyboardInput("<D-A->"));
  dispatch(KeyboardInput("b"));
  runEffects();

  wait(~name="Buffer is correct", (state: State.t) =>
    switch (Selectors.getActiveBuffer(state)) {
    | None => false
    | Some(buf) =>
      let line1 = Buffer.getLine(0, buf) |> BufferLine.raw;
      Log.info("Line1 is: " ++ line1 ++ "|");
      String.equal(line1, "b");
    }
  );
});
