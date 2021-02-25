open Oni_Core;
open Oni_Core.Utility;
open Oni_Model;
open Oni_IntegrationTestLib;

runTest(~name="ClipboardyypTest", ({dispatch, wait, runEffects, _}) => {
  wait(~name="Set configuration to always yank to clipboard", _ => {
    let transformer =
      ConfigurationTransformer.setField(
        "vim.useSystemClipboard",
        `Bool(true),
      );
    dispatch(
      Configuration(Feature_Configuration.Testing.transform(transformer)),
    );
    runEffects();
    true;
  });

  wait(~name="Wait for configuration to update", (state: State.t) => {
    Feature_Configuration.Legacy.getValue(
      c => c.vimUseSystemClipboard,
      state.config,
    )
    == Feature_Configuration.LegacyConfigurationValues.{
         yank: true,
         delete: true,
         paste: true,
       }
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

  dispatch(KeyboardInput({isText: true, input: "y"}));
  dispatch(KeyboardInput({isText: true, input: "y"}));
  dispatch(KeyboardInput({isText: true, input: "p"}));

  runEffects();

  wait(
    ~name="Verify first line gets duplicated", ~timeout=10.0, (state: State.t) => {
    state
    |> Selectors.getActiveBuffer
    |> Option.map(buffer => {
         let firstLine = Buffer.getLine(0, buffer) |> BufferLine.raw;
         let secondLine = Buffer.getLine(1, buffer) |> BufferLine.raw;

         String.equal(firstLine, "abc") && String.equal(secondLine, "abc");
       })
    |> Option.value(~default=false)
  });
});
