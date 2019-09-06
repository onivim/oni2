open Oni_Core;
open Oni_Model;
open Oni_IntegrationTestLib;

let fontToUse = switch (Revery.Environment.os) {
| Revery.Environment.Windows => "Consolas"
| Revery.Environment.Mac => "Menlo"
| Revery.Environment.Linux => "Ubuntu Mono"
| _ => "Courier"
}

// Try loading a different font
let configuration = {|{ "editor.fontFamily": "|} ++ fontToUse ++ {|"}|};

runTest(
  ~configuration=Some(configuration),
  ~name="EditorFontValid",
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
      ~name="There should be a new font set",
      ~timeout=10.,
      (state: State.t) =>

        

      !String.equal(
        state.editorFont.fontFile,
        Utility.executingDirectory ++ "FiraCode-Regular.ttf",
      )
    );
  },
);
