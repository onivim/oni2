open Oni_Core;
open Oni_Model;
open Oni_IntegrationTestLib;

runTest(
  ~name="InsertMode test - effects batched to runEffects",
  ({wait, input, key, _}) => {
  wait(~name="Initial mode is normal", (state: State.t) =>
    Selectors.mode(state) |> Vim.Mode.isNormal
  );

  input("i");

  wait(~name="Mode switches to insert", (state: State.t) =>
    Selectors.mode(state) |> Vim.Mode.isInsert
  );

  input("a");
  input("b");
  input("c");
  input("a");

  wait(~name="Buffer is updated", (state: State.t) =>
    switch (Selectors.getActiveBuffer(state)) {
    | None => false
    | Some(buf) =>
      let str = buf |> Buffer.getLine(0) |> BufferLine.raw;

      str == "abca";
    }
  );

  key(EditorInput.Key.Escape);

  wait(~name="Mode is back to normal", (state: State.t) =>
    Selectors.mode(state) |> Vim.Mode.isNormal
  );

  input("/");
  input("a");
  key(EditorInput.Key.Return);

  wait(
    ~timeout=10.0,
    ~name="Buffer has search highlights for 'a'",
    (state: State.t) =>
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

  input(":");
  input("nohl");
  key(EditorInput.Key.Return);

  wait(
    ~timeout=10.0,
    ~name="Buffer search highlights are cleared",
    (state: State.t) =>
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
