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
  ~name="Regression3048CommandLoopTest",
  ({wait, input, staysTrue, _}) => {
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

    input("i");
    wait(~name="Mode is insert", (state: State.t) =>
      Selectors.mode(state) |> Vim.Mode.isInsert
    );

    // With #3084, we had an infinite cycle with the command -
    // in this case, with our `jj` key binding, it would cause
    // `vim.esc` to be continually dispatched.
    // So to exercise this case - we need to make sure we don't switch back a mode.

    staysTrue(
      ~timeout=1.0, ~name="Should stay in insert mode", (state: State.t) =>
      Selectors.mode(state) |> Vim.Mode.isInsert
    );
  },
);
