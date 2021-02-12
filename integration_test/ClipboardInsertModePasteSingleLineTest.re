open Oni_Core;
open Oni_Model;
open Oni_IntegrationTestLib;

module Log = (
  val Log.withNamespace("IntegrationTest.ClipboardInsertModePasteSingleLine")
);

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

  setClipboard(Some("def"));

  /* Simulate multiple events getting dispatched before running effects */
  dispatch(KeyboardInput({isText: true, input: "A"}));
  wait(~name="Should be a line available", (state: State.t) => {
    switch (Selectors.getActiveBuffer(state)) {
    | None => false
    | Some(buf) => Buffer.getNumberOfLines(buf) >= 1
    }
  });
  dispatch(Actions.Clipboard(Feature_Clipboard.Msg.paste));
  wait(~name="Paste goes through", (state: State.t) =>
    switch (Selectors.getActiveBuffer(state)) {
    | None => false
    | Some(buf) =>
      let line = Buffer.getLine(0, buf) |> BufferLine.raw;
      Log.info("Current line is: |" ++ line ++ "|");
      String.equal(line, "Adef");
    }
  );
  dispatch(KeyboardInput({isText: true, input: "B"}));

  runEffects();

  wait(~name="Buffer shows AdefB", (state: State.t) =>
    switch (Selectors.getActiveBuffer(state)) {
    | None => false
    | Some(buf) =>
      let line = Buffer.getLine(0, buf) |> BufferLine.raw;
      Log.info("Current line is: |" ++ line ++ "|");
      String.equal(line, "AdefB");
    }
  );
});
