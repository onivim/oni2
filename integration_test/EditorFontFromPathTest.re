open Oni_Model;
open Oni_IntegrationTestLib;

let font =
  Sys.getcwd()
  ++ "/"
  ++ getAssetPath("Inconsolata-Regular.ttf")
  |> String.split_on_char('\\')
  |> String.concat("/");

// Try loading a font from a path
let configuration = {|{ "editor.fontFamily": "|} ++ font ++ {|"}|};

runTest(
  ~configuration=Some(configuration),
  ~name="EditorFontFromPath",
  (_, wait, _) => {
    wait(~name="Initial mode is normal", (state: State.t) =>
      Feature_Vim.mode(state.vim) == Vim.Types.Normal
    );

    print_endline("Using font: " ++ font);

    wait(
      ~name="The new font should be set",
      ~timeout=10.,
      (state: State.t) => {
        let currentFontName =
          state.editorFont.fontFamily
          |> Revery_Font.Family.toSkia(Revery_Font.Weight.Normal)
          |> Option.map(tf => Skia.Typeface.getFamilyName(tf))
          |> Option.value(~default="");
        let expectedFontName = "Inconsolata";
        print_endline("Current font is: " ++ currentFontName);
        print_endline("Expected font is: " ++ expectedFontName);
        String.equal(currentFontName, expectedFontName);
      },
    );
  },
);
