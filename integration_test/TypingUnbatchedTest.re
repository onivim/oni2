open Oni_Core;
open Oni_Model;
open Oni_IntegrationTestLib;

module Log = (val Log.withNamespace("IntegrationTest.TypingUnbatched"));

runTest(
  ~name="InsertMode test - effects batched to runEffects",
  (dispatch, wait, runEffects) => {
  wait(~name="Initial mode is normal", (state: State.t) =>
    Feature_Vim.mode(state.vim) == Vim.Types.Normal
  );

  dispatch(KeyboardInput("i"));

  wait(~name="Mode switches to insert", (state: State.t) =>
    Feature_Vim.mode(state.vim) == Vim.Types.Insert
  );

  /* Simulate multiple events getting dispatched before running effects */

  dispatch(KeyboardInput("A"));
  runEffects();

  dispatch(KeyboardInput("B"));
  runEffects();

  dispatch(KeyboardInput("C"));
  runEffects();

  wait(~name="Buffer shows ABC", (state: State.t) =>
    switch (Selectors.getActiveBuffer(state)) {
    | None => false
    | Some(buf) =>
      let line = Buffer.getLine(0, buf) |> BufferLine.raw;
      Log.info("Current line is: |" ++ line ++ "|");
      String.equal(line, "ABC");
    }
  );
});
