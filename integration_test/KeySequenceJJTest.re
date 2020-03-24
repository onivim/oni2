open Oni_Model;
open Oni_IntegrationTestLib;

let keybindings = Some({|
[
  {"key": "jj", "command": "vim.esc", "when": "insertMode"}
]
|});

runTestWithInput(
  ~keybindings,
  ~name="KeySequenceJJTest",
  (input, _dispatch, wait, _) => {
    wait(~name="Initial mode is normal", (state: State.t) =>
      state.mode == Vim.Types.Normal
    );

    input("i");
    wait(~name="Mode is now insert", (state: State.t) =>
      state.mode == Vim.Types.Insert
    );
    
    input("j");
    input("j");
    
    wait(~name="Mode is back to normal", (state: State.t) =>
      state.mode == Vim.Types.Normal
    );
  },
);
