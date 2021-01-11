open Oni_Model;
open Oni_IntegrationTestLib;
open Actions;

let keybindings =
  Some({|
[
  {"key": "k", "command": ":norm! j", "when": "normalMode"}
]
|});

runTest(
  ~keybindings,
  ~name="ExCommandKeybindingNormTest",
  (dispatch, wait, _) => {
    let input = key => {
      let keyPress =
        EditorInput.KeyPress.physicalKey(
          ~key,
          ~modifiers=EditorInput.Modifiers.none,
        )
        |> EditorInput.KeyCandidate.ofKeyPress;
      let time = Revery.Time.now();

      dispatch(KeyDown({key: keyPress, scancode: 1, time}));
      //dispatch(TextInput(key));
      dispatch(KeyUp({scancode: 1, time}));
    };

    let testFile = getAssetPath("some-test-file.txt");
    dispatch(Actions.OpenFileByPath(testFile, None, None));

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
    input(EditorInput.Key.Character('k'));

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
