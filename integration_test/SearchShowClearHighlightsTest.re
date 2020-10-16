open Oni_Core;
open Oni_Model;
open Oni_IntegrationTestLib;
open Oni_Syntax;

runTest(
  ~name="InsertMode test - effects batched to runEffects",
  (dispatch, wait, runEffects) => {
  wait(~name="Initial mode is normal", (state: State.t) =>
    Feature_Vim.mode(state.vim) |> Vim.Mode.isNormal
  );

  // Edit
  dispatch(KeyboardInput({isText: true, input: "i"}));

  wait(~name="Mode switches to insert", (state: State.t) =>
    Feature_Vim.mode(state.vim) |> Vim.Mode.isInsert
  );

  dispatch(KeyboardInput({isText: true, input: "a"}));
  dispatch(KeyboardInput({isText: true, input: "b"}));
  dispatch(KeyboardInput({isText: true, input: "c"}));

  dispatch(KeyboardInput({isText: false, input: "<esc>"}));

  dispatch(KeyboardInput({isText: true, input: "/"}));
  dispatch(KeyboardInput({isText: true, input: "a"}));
  dispatch(KeyboardInput({isText: false, input: "<cr>"}));

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

  dispatch(KeyboardInput({isText: true, input: ":"}));
  dispatch(KeyboardInput({isText: true, input: "nohl"}));
  dispatch(KeyboardInput({isText: false, input: "<cr>"}));

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
