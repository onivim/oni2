open Oni_Model;
open Oni_IntegrationTestLib;

let fontToUse =
  switch (Revery.Environment.os) {
  | Revery.Environment.Windows => "Consolas"
  | Revery.Environment.Mac => "Menlo"
  | Revery.Environment.Linux => "Ubuntu Mono"
  | _ => "Courier"
  };

// Try loading a different font
let configuration = {|{ "editor.fontFamily": "|} ++ fontToUse ++ {|"}|};

if (Revery.Environment.os !== Revery.Environment.Linux) {
  // Skipping this test on Linux, because there is a known fontconfig memory leak we hit.
  // The `FcInit` function in fontconfig leaks, and calling `FcFini` to clean it up crashes.
  // Some related notes:
  // https://www.spinics.net/lists/font-config/msg04332.html
  // https://bugs.archlinux.org/task/64168
  runTest(
    ~configuration=Some(configuration),
    ~name="EditorFontValid",
    (_, wait, _) => {
      wait(~name="Initial mode is normal", (state: State.t) =>
        Feature_Vim.mode(state.vim) == Vim.Types.Normal
      );

      wait(
        ~name="There should be a new font set", ~timeout=10., (state: State.t) =>
        !
          String.equal(
            state.editorFont.fontFile,
            Revery.Environment.executingDirectory ++ "FiraCode-Regular.ttf",
          )
      );
    },
  );
};
