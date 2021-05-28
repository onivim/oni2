open Oni_Model;
open Oni_IntegrationTestLib;

let keybindings =
  Some({|
[
  {"key": "k", "command": ":norm! j", "when": "normalMode"}
]
|});

runTest(
  ~keybindings,
  ~name="ExCommandKeybindingNormTest",
  ({dispatch, wait, input, _}) => {
    let testFile = getAssetPath("some-test-file.txt");
    dispatch(
      Actions.OpenFileByPath(testFile, Oni_Core.SplitDirection.Current, None),
    );

    wait(~name="Verify buffer is loaded", (state: State.t) => {
      state
      |> Selectors.getActiveBuffer
      |> Option.map(Oni_Core.Buffer.getNumberOfLines)
      |> Option.map(lineCount => lineCount > 1)
      |> Option.value(~default=false)
    });

    // Verify cursor is at top of file
    wait(~name="Verify cursor at top of file", (state: State.t) => {
      let cursor =
        state.layout
        |> Feature_Layout.activeEditor
        |> Feature_Editor.Editor.getPrimaryCursor;

      EditorCoreTypes.(cursor.line == LineNumber.zero);
    });

    // Press k, which is re-bound to 'norm! j'
    input("k");

    // Verify cursor is at top of file
    wait(~name="Verify cursor moved down a line", (state: State.t) => {
      let cursor =
        state.layout
        |> Feature_Layout.activeEditor
        |> Feature_Editor.Editor.getPrimaryCursor;

      EditorCoreTypes.(cursor.line == LineNumber.(zero + 1));
    });
  },
);
