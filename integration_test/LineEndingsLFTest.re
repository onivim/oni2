open Oni_Core;
open Oni_Core.Utility;
open Oni_Model;
open Oni_IntegrationTestLib;

runTest(~name="LineEndingsLFTest", ({dispatch, wait, _}) => {
  wait(~name="Capture initial state", (state: State.t) =>
    Selectors.mode(state) |> Vim.Mode.isNormal
  );

  let testFile = getAssetPath("test.lf");

  // Create a buffer
  dispatch(Actions.OpenFileByPath(testFile, SplitDirection.Current, None));

  wait(~name="Verify buffer is loaded", (state: State.t) => {
    state
    |> Selectors.getActiveBuffer
    |> OptionEx.flatMap(Buffer.getShortFriendlyName)
    |> Option.map(name => String.equal(name, "test.lf"))
    |> Option.value(~default=false)
  });

  wait(~name="Verify buffer is loaded as LF", ~timeout=10.0, (state: State.t) => {
    state
    |> Selectors.getActiveBuffer
    |> OptionEx.flatMap(Buffer.getLineEndings)
    |> Option.map(lineEndings => lineEndings == Vim.Types.LF)
    |> Option.value(~default=false)
  });

  // Switch line endings
  dispatch(
    Actions.VimExecuteCommand({allowAnimation: true, command: "set ff=dos"}),
  );

  wait(
    ~name="Verify buffer is switched to CRLF", ~timeout=10.0, (state: State.t) => {
    state
    |> Selectors.getActiveBuffer
    |> OptionEx.flatMap(Buffer.getLineEndings)
    |> Option.map(lineEndings => lineEndings == Vim.Types.CRLF)
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
