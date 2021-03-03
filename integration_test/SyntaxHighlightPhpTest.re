open Oni_Core;
open Oni_Model;
open Oni_IntegrationTestLib;

let uniqueColorCount: list(ThemeToken.t) => int =
  tokens => {
    let rec loop = (uniqueCount, maybeLastForeground, tokens) => {
      switch (tokens) {
      | [] => uniqueCount
      | [hd, ...tail] =>
        let hd: ThemeToken.t = hd;
        switch (maybeLastForeground) {
        | None => loop(uniqueCount + 1, Some(hd.foregroundColor), tail)
        | Some(color) =>
          if (Revery.Color.equals(color, hd.foregroundColor)) {
            loop(uniqueCount, Some(hd.foregroundColor), tail);
          } else {
            loop(uniqueCount + 1, Some(hd.foregroundColor), tail);
          }
        };
      };
    };

    loop(0, None, tokens);
  };

// Integration test - verify we get highlights for PHP
// Regression test for #2985
runTest(~name="SyntaxHighlightPhpTest", ({dispatch, wait, _}) => {
  wait(~name="Capture initial state", (state: State.t) =>
    Selectors.mode(state) |> Vim.Mode.isNormal
  );
  wait(~name="Wait for syntax server", ~timeout=10.0, (state: State.t) => {
    Feature_Syntax.isSyntaxServerRunning(state.syntaxHighlights)
  });

  let testFile = getAssetPath("test-syntax.php");

  // Create a buffer
  dispatch(Actions.OpenFileByPath(testFile, None, None));

  // Wait for highlights to show up
  wait(~name="Verify we get syntax highlights", (state: State.t) => {
    state
    |> Selectors.getActiveBuffer
    |> Option.map(Buffer.getId)
    |> Option.map(bufferId => {
         let tokens =
           Feature_Syntax.getTokens(
             ~bufferId,
             ~line=EditorCoreTypes.LineNumber.(zero),
             state.syntaxHighlights,
           );

         let uniqueTokenColors = tokens |> uniqueColorCount;

         uniqueTokenColors > 1;
       })
    |> Option.value(~default=false)
  });
});
