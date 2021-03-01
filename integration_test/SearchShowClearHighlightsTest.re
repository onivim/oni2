open Oni_Core;
open Oni_Model;
open Oni_IntegrationTestLib;

runTest(
  ~name="InsertMode test - effects batched to runEffects",
  ({dispatch, wait, runEffects, input, key, _}) => {
  wait(~name="Initial mode is normal", (state: State.t) =>
    Selectors.mode(state) |> Vim.Mode.isNormal
  );

  // Edit
  //dispatch(KeyboardInput({isText: true, input: "i"}));
  input("i");

  wait(~name="Mode switches to insert", (state: State.t) =>
    Selectors.mode(state) |> Vim.Mode.isInsert
  );

  input("a");
  input("b");
  input("c");

  wait(~name="Buffer is updated", (state: State.t) =>
    switch (Selectors.getActiveBuffer(state)) {
    | None => false
    | Some(buf) =>
      let str = buf
      |> Buffer.getLine(0)
      |> BufferLine.raw;

      str == "abc";
    }
  );

  key(EditorInput.Key.Escape);

  wait(~name="Mode is back to normal", (state: State.t) =>
    Selectors.mode(state) |> Vim.Mode.isNormal
  );

  // dispatch(KeyboardInput({isText: true, input: "a"}));
  // dispatch(KeyboardInput({isText: true, input: "b"}));
  // dispatch(KeyboardInput({isText: true, input: "c"}));

  // dispatch(KeyboardInput({isText: false, input: "<esc>"}));

  input("/");
  input("a");
  key(EditorInput.Key.Return);
  // dispatch(KeyboardInput({isText: true, input: "/"}));
  // dispatch(KeyboardInput({isText: true, input: "a"}));
  // dispatch(KeyboardInput({isText: false, input: "<cr>"}));

  // runEffects();

  wait(~timeout=10.0, ~name="Buffer has search highlights for 'a'", (state: State.t) =>
    switch (Selectors.getActiveBuffer(state)) {
    | None => false
    | Some(buf) =>
      let bufferId = Buffer.getId(buf);
      let searchHighlightCount =
        Feature_Vim.getSearchHighlightsByLine(
          ~bufferId,
          ~line=EditorCoreTypes.LineNumber.zero,
          state.vim,
        )
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
        Feature_Vim.getSearchHighlightsByLine(
          ~bufferId,
          ~line=EditorCoreTypes.LineNumber.zero,
          state.vim,
        )
        |> List.length;
      searchHighlightCount == 0;
    }
  );
});
