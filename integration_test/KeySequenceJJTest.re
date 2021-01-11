open Oni_Model;
open Oni_IntegrationTestLib;

let keybindings =
  Some({|
[
  {"key": "jj", "command": "vim.esc", "when": "insertMode"}
]
|});

runTest(
  ~keybindings,
  ~name="KeySequenceJJTest",
  ({wait, input, _}) => {
    wait(~name="Initial mode is normal", (state: State.t) =>
      Selectors.mode(state) |> Vim.Mode.isNormal
    );

    input("i");
    wait(~name="Mode is now insert", (state: State.t) =>
      Selectors.mode(state) |> Vim.Mode.isInsert
    );

    input("a");
    input("j");
    input("j");

    wait(~name="Mode is back to normal", (state: State.t) =>
      Selectors.mode(state) |> Vim.Mode.isNormal
    );

    wait(~name="Validate buffer is empty", (state: State.t) => {
      let actual =
        Model.Selectors.getActiveBuffer(state)
        |> Option.map(Core.Buffer.getLines)
        |> Option.map(Array.to_list);

      actual == Some(["a"]);
    });

    wait(~name="#2601: Validate editor mode is normal, too", (state: State.t) => {
      let editorMode =
        state.layout
        |> Feature_Layout.activeEditor
        |> Feature_Editor.Editor.mode;

      Vim.Mode.isNormal(editorMode);
    });

    // #2601 - Make sure we're _actually_ in normal mode!
    // Type another 'j' to see...
    input("j");

    wait(
      ~name=
        "#2601: Buffer should _still_ be empty, since we used j in normal mode",
      (state: State.t) => {
      let actual =
        Model.Selectors.getActiveBuffer(state)
        |> Option.map(Core.Buffer.getLines)
        |> Option.map(Array.to_list);

      actual == Some(["a"]);
    });
  },
);
