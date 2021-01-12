open Oni_Core;
open Oni_Model;
open Oni_IntegrationTestLib;

module Log = (
  val Log.withNamespace("IntegrationTest.ClipboardInsertModePasteEmpty")
);

runTest(
  ~name="InsertMode test - effects batched to runEffects",
  (dispatch, wait, runEffects) => {
  wait(~name="Initial mode is normal", (state: State.t) =>
    Selectors.mode(state) |> Vim.Mode.isNormal
  );

  dispatch(KeyboardInput({isText: true, input: "i"}));

  wait(~name="Mode switches to insert", (state: State.t) =>
    Selectors.mode(state) |> Vim.Mode.isInsert
  );

  setClipboard(None);

  /* Simulate multiple events getting dispatched before running effects */
  dispatch(KeyboardInput({isText: true, input: "A"}));
  dispatch(Command("editor.action.clipboardPasteAction"));
  dispatch(KeyboardInput({isText: true, input: "B"}));

  runEffects();

  wait(~name="Buffer shows AB", (state: State.t) =>
    switch (Selectors.getActiveBuffer(state)) {
    | None => false
    | Some(buf) =>
      let line = Buffer.getLine(0, buf) |> BufferLine.raw;
      Log.info("Current line is: |" ++ line ++ "|");
      String.equal(line, "AB");
    }
  );
});
