open EditorCoreTypes;

open Oni_Core;
open Oni_Core.Utility;
open Oni_Model;
open Oni_IntegrationTestLib;

let configuration = Some({|
{ "experimental.treeSitter": true }
|});

// Validate that treesitter highlight runs
runTest(
  ~configuration,
  ~name="SyntaxHighlightTreesitterTest",
  (dispatch, wait, _runEffects) => {
    wait(~name="Capture initial state", (state: State.t) =>
      state.mode == Vim.Types.Normal
    );

    let testFile = getAssetPath("some-test-file.json");

    // Create a buffer
    dispatch(Actions.OpenFileByPath(testFile, None, None));

    // Wait for highlights to show up
    wait(~name="Verify we get syntax highlights", (state: State.t) => {
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
  },
);
