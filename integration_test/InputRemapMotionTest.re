open Oni_Model;
open Oni_IntegrationTestLib;
open EditorCoreTypes;

runTest(
  ~name="InputRemapMotionTest (#2114)",
  ({dispatch, wait, runEffects, input, _}) => {
  wait(~name="Initial mode is normal", (state: State.t) =>
    Selectors.mode(state) |> Vim.Mode.isNormal
  );

  let testFile = getAssetPath("some-test-file.txt");
  // Open an erroneous CSS file - verify we get some diagnostics
  dispatch(
    Actions.OpenFileByPath(testFile, Oni_Core.SplitDirection.Current, None),
  );

  wait(~name="buffer load", (state: State.t) => {
    switch (Selectors.getActiveBuffer(state)) {
    | Some(buffer) => Oni_Core.Buffer.getNumberOfLines(buffer) > 2
    | None => false
    }
  });

  // #2114 - remap hjkl -> jklm
  dispatch(
    VimExecuteCommand({allowAnimation: true, command: "nnoremap j h "}),
  );
  runEffects();

  dispatch(
    VimExecuteCommand({allowAnimation: true, command: "nnoremap l k "}),
  );
  runEffects();

  dispatch(
    VimExecuteCommand({allowAnimation: true, command: "nnoremap m l "}),
  );
  runEffects();

  dispatch(
    VimExecuteCommand({allowAnimation: true, command: "nnoremap k j "}),
  );
  runEffects();

  let waitForCursorPosition = bytePosition => {
    wait(
      ~name="Verify cursor is at:" ++ BytePosition.show(bytePosition),
      (state: State.t) => {
        let cursorPosition =
          state.layout
          |> Feature_Layout.activeEditor
          |> Feature_Editor.Editor.getPrimaryCursorByte;

        cursorPosition == bytePosition;
      },
    );
  };

  waitForCursorPosition(
    BytePosition.{line: LineNumber.zero, byte: ByteIndex.zero},
  );

  // Move down, jklm style
  input("k");
  waitForCursorPosition(
    BytePosition.{line: LineNumber.(zero + 1), byte: ByteIndex.zero},
  );

  // Move right, jklm style
  input("m");
  waitForCursorPosition(
    BytePosition.{line: LineNumber.(zero + 1), byte: ByteIndex.(zero + 1)},
  );

  // Move up, jklm style
  input("l");
  waitForCursorPosition(
    BytePosition.{line: LineNumber.(zero), byte: ByteIndex.(zero + 1)},
  );

  // Move left, jklm style
  input("j");
  waitForCursorPosition(
    BytePosition.{line: LineNumber.(zero), byte: ByteIndex.(zero)},
  );
});
