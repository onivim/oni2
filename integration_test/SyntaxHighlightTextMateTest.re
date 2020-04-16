open EditorCoreTypes;

open Oni_Core;
open Oni_Model;
open Oni_IntegrationTestLib;

// Validate that textmate highlight runs
runTest(~name="SyntaxHighlightTextMateTest", (dispatch, wait, _runEffects) => {
  wait(~name="Capture initial state", (state: State.t) =>
    state.vimMode == Vim.Types.Normal
  );

  let testFile = getAssetPath("large-c-file.c");

  // Create a buffer
  dispatch(Actions.OpenFileByPath(testFile, None, None));

  // Wait for highlights to show up
  wait(
    ~name="Verify we get syntax highlights", ~timeout=30.0, (state: State.t) => {
    state
    |> Selectors.getActiveBuffer
    |> Option.map(Buffer.getId)
    |> Option.map(bufferId => {
         let tokens =
           Feature_Syntax.getTokens(
             ~bufferId,
             // Verify we get highlighting at the end!
             ~line=Index.(zero+1410),
             state.syntaxHighlights,
           );

         List.length(tokens) > 1;
       })
    |> Option.value(~default=false)
  });
});
