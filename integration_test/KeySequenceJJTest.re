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
      Feature_Vim.mode(state.vim) == Vim.Types.Normal
    );

    let input = key => {
      let scancode = Sdl2.Scancode.ofName(key);
      let keycode = Sdl2.Keycode.ofName(key);
      let modifiers = EditorInput.Modifiers.none;

      let keyPress: EditorInput.KeyPress.t = {scancode, keycode, modifiers};
      let time = Revery.Time.now();

      dispatch(Model.Actions.KeyDown(keyPress, time));
      //dispatch(Model.Actions.TextInput(key));
      dispatch(Model.Actions.KeyUp(keyPress, time));
    };

    input("i");
    wait(~name="Mode is now insert", (state: State.t) =>
      Feature_Vim.mode(state.vim) == Vim.Types.Insert
    );

    input("j");
    input("j");

    wait(~name="Mode is back to normal", (state: State.t) =>
      Feature_Vim.mode(state.vim) == Vim.Types.Normal
    );
    // TODO: Figure out why this check is failing...
    wait(~name="Validate buffer is empty", (state: State.t) => {
      Model.Selectors.getActiveBuffer(state)
      |> Option.map(Core.Buffer.getLines)
      |> Option.map(Array.to_list) == Some([""])
    });
  },
);
