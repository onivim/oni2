open Oni_Core;
open Oni_Model;
open Oni_IntegrationTestLib;

let font = 
    (Utility.executingDirectory ++ "Inconsolata-Regular.ttf")
    |> String.split_on_char('\\')
    |> String.concat("/");

// Try loading a font from a path
let configuration = {|{ "editor.fontFamily": "|} ++ font ++ {|"}|};

runTest(
  ~configuration=Some(configuration),
  ~name="EditorFontFromPath",
  (_, wait, _) => {
    wait(~name="Initial mode is normal", (state: State.t) =>
      state.mode == Vim.Types.Normal
    );
    
    wait(
      ~name="Font should initially be set to default",
      ~timeout=10.,
      (state: State.t) =>
      String.equal(
        state.editorFont.fontFile,
        Utility.executingDirectory ++ "FiraCode-Regular.ttf",
      )
    );

    wait(
      ~name="The new font should be set",
      ~timeout=10.,
      (state: State.t) =>

      String.equal(
        state.editorFont.fontFile,
        font,
      )
    );
  },
);
