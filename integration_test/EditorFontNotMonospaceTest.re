open Oni_Core;
open Oni_Model;
open Oni_IntegrationTestLib;

// Arial is a non-monospace font that is available on all the platforms we test
let configuration = {|
{ "editor.fontFamily": "Arial" }
|};

// This validates a couple things:
// - When we try to load a font that is not monospace, we get a notification
// - When we try to load a font that is not monospace, we still have the default font
runTest(
  ~configuration=Some(configuration),
  ~name="EditorFontNotMonospace", 
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
    String.equal(state.editorFont.fontFile, Utility.executingDirectory ++ "FiraCode-Regular.ttf");
  });
});
