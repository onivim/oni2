open Oni_Core;
open Oni_Core.Utility;
open Oni_Model;
open Oni_IntegrationTestLib;

runTest(~name="LineEndingsCRLFTest", (dispatch, wait, _runEffects) => {
  wait(~name="Capture initial state", (state: State.t) =>
    state.vimMode == Vim.Types.Normal
  );

  let testFile = getAssetPath("test.crlf");

  // Create a buffer
  dispatch(Actions.OpenFileByPath(testFile, None, None));

  // Wait for highlights to show up
  wait(
    ~name="Verify buffer is loaded as CRLF", ~timeout=10.0, (state: State.t) => {
    state
    |> Selectors.getActiveBuffer
    |> OptionEx.flatMap(Buffer.getLineEndings)
    |> Option.map(lineEndings => lineEndings == LineEnding.CRLF)
    |> Option.value(~default=false)
  });

  // Switch line endings
  dispatch(Actions.VimExecuteCommand("set ff=unix"));

  // Verify buffer now has LF line endings...
  wait(
    ~name="Verify buffer is switched to LF", ~timeout=10.0, (state: State.t) => {
    state
    |> Selectors.getActiveBuffer
    |> OptionEx.flatMap(Buffer.getLineEndings)
    |> Option.map(lineEndings => lineEndings == LineEnding.LF)
    |> Option.value(~default=false)
  });

  // ...and is modified
  wait(~name="Verify buffer is modified", ~timeout=10.0, (state: State.t) => {
    state
    |> Selectors.getActiveBuffer
    |> Option.map(Buffer.isModified)
    |> Option.value(~default=false)
  });
});
