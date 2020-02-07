open EditorCoreTypes;

open Oni_Core;
open Oni_Model;
open Oni_IntegrationTestLib;

module BufferSyntaxHighlights = Feature_Editor.BufferSyntaxHighlights;

let gotSyntaxServerClose = ref(false);

let onAfterDispatch =
  fun
  | Actions.SyntaxServerClosed => gotSyntaxServerClose := true
  | _ => ();

// Validate that textmate highlight runs
runTestWithInput(
  ~onAfterDispatch,
  ~name="Regression1097Test",
  (input, dispatch, wait, _runEffects) => {
    wait(~name="Capture initial state", (state: State.t) =>
      state.mode == Vim.Types.Normal
    );

    let testFile = getAssetPath("some-test-file.json");

    // Create a buffer
    dispatch(Actions.OpenFileByPath(testFile, None, None));

    let i = ref(0);
    while (i^ < 25) {
      input("j");
      input("k");
      incr(i);
    };

    // Open file again
    dispatch(Actions.OpenFileByPath(testFile, None, None));

    // Wait for highlights to show up
    wait(
      ~name="Verify we get syntax highlights", ~timeout=10.0, (state: State.t) => {
      state
      |> Selectors.getActiveBuffer
      |> Option.map(Buffer.getId)
      |> Option.map(bufferId => {
           let tokens =
             BufferSyntaxHighlights.getTokens(
               bufferId,
               Index.zero,
               state.bufferSyntaxHighlights,
             );

           List.length(tokens) > 1;
         })
      |> Option.value(~default=false)
    });

    // Verify syntax server did not close
    wait(~name="Verify we get syntax highlights", ~timeout=10.0, _state => {
      ! gotSyntaxServerClose^
    });
  },
);
