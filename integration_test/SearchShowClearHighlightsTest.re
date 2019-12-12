open Oni_Core;
open Oni_Model;
open Oni_IntegrationTestLib;

runTest(
  ~name="InsertMode test - effects batched to runEffects",
  (dispatch, wait, runEffects) => {
  wait(~name="Initial mode is normal", (state: State.t) =>
    state.mode == Vim.Types.Normal
  );

  // Edit
  dispatch(KeyboardInput("i"));

  wait(~name="Mode switches to insert", (state: State.t) =>
    state.mode == Vim.Types.Insert
  );

  dispatch(KeyboardInput("a"));
  dispatch(KeyboardInput("b"));
  dispatch(KeyboardInput("c"));

  dispatch(KeyboardInput("<esc>"));

  dispatch(KeyboardInput("/"));
  dispatch(KeyboardInput("a"));
  dispatch(KeyboardInput("<cr>"));

  runEffects();

  wait(~name="Buffer has search highlights for 'a'", (state: State.t) =>
    switch (Selectors.getActiveBuffer(state)) {
    | None => false
    | Some(buf) =>
      let bufferId = Buffer.getId(buf);
      let searchHighlightCount =
        BufferHighlights.getHighlights(~bufferId, state.bufferHighlights)
        |> List.length;
      searchHighlightCount > 0;
    }
  );

  dispatch(KeyboardInput(":"));
  dispatch(KeyboardInput("n"));
  dispatch(KeyboardInput("o"));
  dispatch(KeyboardInput("h"));
  dispatch(KeyboardInput("l"));
  dispatch(KeyboardInput("<cr>"));
  runEffects();

  wait(~name="Buffer search highlights are cleared", (state: State.t) =>
    switch (Selectors.getActiveBuffer(state)) {
    | None => false
    | Some(buf) =>
      let bufferId = Buffer.getId(buf);
      let searchHighlightCount =
        BufferHighlights.getHighlights(~bufferId, state.bufferHighlights)
        |> List.length;
      searchHighlightCount == 0;
    }
  );
});
