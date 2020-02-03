open Oni_Model;
open Oni_IntegrationTestLib;

let font = 
  Sys.getcwd() ++ "/" ++ getAssetPath("Inconsolata-Regular.ttf")
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

    print_endline("Using font: " ++ font);

    wait(~name="The new font should be set", ~timeout=10., (state: State.t) => {
      print_endline ("Current font is: " ++ state.editorFont.fontFile);
      print_endline ("Expected font is: " ++ font);
      String.equal(state.editorFont.fontFile, font)
    });
  },
);
