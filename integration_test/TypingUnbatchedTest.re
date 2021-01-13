open Oni_Core;
open Oni_Model;
open Oni_IntegrationTestLib;

module Log = (val Log.withNamespace("IntegrationTest.TypingUnbatched"));

runTest(
  ~name="InsertMode test - effects batched to runEffects",
  ({dispatch, wait, runEffects, _}) => {
  wait(~name="Initial mode is normal", (state: State.t) =>
    Selectors.mode(state) |> Vim.Mode.isNormal
  );

  dispatch(KeyboardInput({isText: true, input: "i"}));

  wait(~name="Mode switches to insert", (state: State.t) =>
    Selectors.mode(state) |> Vim.Mode.isInsert
  );

  /* Simulate multiple events getting dispatched before running effects */

  dispatch(KeyboardInput({isText: true, input: "A"}));
  runEffects();

  dispatch(KeyboardInput({isText: true, input: "B"}));
  runEffects();

  dispatch(KeyboardInput({isText: true, input: "C"}));
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
