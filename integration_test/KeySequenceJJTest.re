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
  (dispatch, wait, _) => {
    wait(~name="Initial mode is normal", (state: State.t) =>
      state.mode == Vim.Types.Normal
    );

    let input = key => {
      let scancode = Sdl2.Scancode.ofName(key);
      let keycode = Sdl2.Keycode.ofName(key);
      let modifiers = EditorInput.Modifiers.none;

      let keyPress: EditorInput.keyPress = {scancode, keycode, modifiers};

      dispatch(Model.Actions.KeyDown(keyPress));
      //dispatch(Model.Actions.TextInput(key));
      dispatch(Model.Actions.KeyUp(keyPress));
    };

    input("i");
    wait(~name="Mode is now insert", (state: State.t) =>
      state.mode == Vim.Types.Insert
    );

    input("j");
    input("j");

    wait(~name="Mode is back to normal", (state: State.t) =>
      state.mode == Vim.Types.Normal
    );
    // TODO: Figure out why this check is failing...
    wait(~name="Validate buffer is empty", (state: State.t) => {
      Model.Selectors.getActiveBuffer(state)
      |> Option.map(Core.Buffer.getLines)
      |> Option.map(Array.to_list) == Some([""])
    });
  },
);
