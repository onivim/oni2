open Oni_Core;
open Oni_Core.Utility;
open Oni_Model;
open Oni_IntegrationTestLib;

runTest(~name="ClipboardyypTest", (dispatch, wait, runEffects) => {
  wait(
    ~name="Set configuration to always yank to clipboard", (state: State.t) => {
    let configuration = state.configuration;
    dispatch(
      ConfigurationSet({
        ...configuration,
        default: {
          ...configuration.default,
          vimUseSystemClipboard: {
            yank: true,
            paste: true,
            delete: true,
          },
        },
      }),
    );
    runEffects();
    true;
  });

  let testFile = getAssetPath("some-test-file.txt");
  dispatch(Actions.OpenFileByPath(testFile, None, None));

  wait(~name="Verify buffer is loaded", (state: State.t) => {
    state
    |> Selectors.getActiveBuffer
    |> OptionEx.flatMap(Buffer.getShortFriendlyName)
    |> Option.map(name => String.equal(name, "some-test-file.txt"))
    |> Option.value(~default=false)
  });

  dispatch(KeyboardInput("x"));
  dispatch(KeyboardInput("p"));

  runEffects();

  wait(~name="Verify characters get swapped", ~timeout=10.0, (state: State.t) => {
    state
    |> Selectors.getActiveBuffer
    |> Option.map(buffer => {
         let firstLine = Buffer.getLine(0, buffer) |> BufferLine.raw;

         String.equal(firstLine, "bac");
       })
    |> Option.value(~default=false)
  });
});
