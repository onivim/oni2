open Oni_Core;
open Oni_Model;
open Oni_IntegrationTestLib;

// Try loading a non-existent font...
let configuration = {|
{ "editor.fontFamily": "SomeRandomFontThatDoesntExist" }
|};

runTest(
  ~configuration=Some(configuration),
  ~name="EditorFontInvalid",
  (_, wait, _) => {
    wait(~name="Initial mode is normal", (state: State.t) =>
      state.mode == Vim.Types.Normal
    );

    wait(
      ~name="There should be an error notification for the font",
      ~timeout=10.,
      (state: State.t) =>
      Notifications.any(state.notifications)
    );

    wait(~name="The font should still be the default", (state: State.t) => {
      String.equal(
        state.editorFont.fontFile,
        Utility.executingDirectory ++ "FiraCode-Regular.ttf",
      )
    });
  },
);
