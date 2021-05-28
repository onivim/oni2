open Oni_Model;
open Oni_IntegrationTestLib;

let fontToUse =
  switch (Revery.Environment.os) {
  | Revery.Environment.Windows(_) => "Consolas"
  | Revery.Environment.Mac(_) => "Menlo"
  | Revery.Environment.Linux(_) => "Ubuntu Mono"
  | _ => "Courier"
  };

// Try loading a different font
let configuration = {|{ "editor.fontFamily": "|} ++ fontToUse ++ {|"}|};

if (!Revery.Environment.isLinux) {
  // Skipping this test on Linux, because there is a known fontconfig memory leak we hit.
  // The `FcInit` function in fontconfig leaks, and calling `FcFini` to clean it up crashes.
  // Some related notes:
  // https://www.spinics.net/lists/font-config/msg04332.html
  // https://bugs.archlinux.org/task/64168
  runTest(
    ~configuration=Some(configuration),
    ~name="EditorFontValid",
    ({wait, _}) => {
      wait(~name="Initial mode is normal", (state: State.t) =>
        Selectors.mode(state) |> Vim.Mode.isNormal
      );

      wait(
        ~name="There should be a new font set",
        ~timeout=10.,
        (state: State.t) => {
          let fontName =
            state.editorFont.fontFamily
            |> Revery_Font.Family.toSkia(Revery.Font.Weight.Normal)
            |> Option.map(tf => Skia.Typeface.getFamilyName(tf))
            |> Option.value(~default="");
          !String.equal(fontName, "JetBrains Mono");
        },
      );
    },
  );
};
