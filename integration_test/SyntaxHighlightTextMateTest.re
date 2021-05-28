open Oni_Core;
open Oni_Model;
open Oni_IntegrationTestLib;

// Validate that textmate highlight runs
runTest(~name="SyntaxHighlightTextMateTest", ({dispatch, wait, _}) => {
  wait(~name="Capture initial state", (state: State.t) =>
    Selectors.mode(state) |> Vim.Mode.isNormal
  );
  wait(~name="Wait for syntax server", ~timeout=10.0, (state: State.t) => {
    Feature_Syntax.isSyntaxServerRunning(state.syntaxHighlights)
  });

  let testFile = getAssetPath("large-c-file.c");

  // Create a buffer
  dispatch(Actions.OpenFileByPath(testFile, SplitDirection.Current, None));

  // Wait for highlights to show up
  wait(
    ~name="Verify we get syntax highlights", ~timeout=60.0, (state: State.t) => {
    state
    |> Selectors.getActiveBuffer
    |> Option.map(Buffer.getId)
    |> Option.map(bufferId => {
         let tokens =
           Feature_Syntax.getTokens(
             ~bufferId,
             // Verify we get highlighting at the end!
             ~line=EditorCoreTypes.LineNumber.(zero + 14110),
             state.syntaxHighlights,
           );

         List.length(tokens) > 1;
       })
    |> Option.value(~default=false)
  });
});
