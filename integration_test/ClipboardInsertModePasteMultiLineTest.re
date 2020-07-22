open Oni_Core;
open Oni_Model;
open Oni_IntegrationTestLib;

module Log = (
  val Log.withNamespace("IntegrationTest.ClipboardInsertModePasteMultiLine")
);

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

  setClipboard(Some("def\nghi"));

  dispatch(KeyboardInput("A"));
  dispatch(Actions.Clipboard(Feature_Clipboard.Msg.paste));
  wait(~name="Paste goes through", (state: State.t) =>
    state
    |> Selectors.getActiveBuffer
    |> Option.map(buf => Buffer.getNumberOfLines(buf) == 2)
    |> Option.value(~default=false)
  );

  dispatch(KeyboardInput("B"));

  runEffects();

  wait(~name="Buffer is correct", (state: State.t) =>
    switch (Selectors.getActiveBuffer(state)) {
    | None => false
    | Some(buf) =>
      let line1 = Buffer.getLine(0, buf) |> BufferLine.raw;
      prerr_endline("LINE1: " ++ line1);
      let line2 = Buffer.getLine(1, buf) |> BufferLine.raw;
      Log.info("Line1 is: " ++ line1 ++ "|");
      Log.info("Line2 is: " ++ line2 ++ "|");
      String.equal(line1, "Adef") && String.equal(line2, "ghiB");
    }
  );
});
