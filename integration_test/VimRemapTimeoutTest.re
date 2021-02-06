open Oni_Core;
open Oni_Model;
open Oni_IntegrationTestLib;

runTest(
  ~name="VimRemapTimeoutTest", ({dispatch, wait, runEffects, input, _}) => {
  wait(~name="Initial mode is normal", (state: State.t) =>
    Selectors.mode(state) |> Vim.Mode.isNormal
  );

  // Set low input timeout: 250ms
  dispatch(
    VimExecuteCommand({allowAnimation: true, command: "timeoutlen 250"}),
  );
  runEffects();

  // Use inoremap to set up jk -> <ESC> binding
  dispatch(
    VimExecuteCommand({allowAnimation: true, command: "inoremap jk <ESC>"}),
  );
  runEffects();

  input("i");
  wait(~name="Mode is now insert", (state: State.t) =>
    Selectors.mode(state) |> Vim.Mode.isInsert
  );

  input("j");

  // Verify we are in a 'pending' input state...
  wait(~name="`j` should be consumed", (state: State.t) =>
    Feature_Input.consumedKeys(state.input) != []
  );

  // ... and then the timeout is reached, dropping the `j` into the buffer.
  wait(
    ~timeout=5.0,
    ~name="j should be emitted to buffer",
    (state: State.t) => {
      let buffer = Selectors.getActiveBuffer(state) |> Option.get;

      let line =
        if (Buffer.getNumberOfLines(buffer) > 0) {
          buffer |> Buffer.getLine(0) |> BufferLine.raw;
        } else {
          "";
        };

      // Verify character
      line == "j" && Feature_Input.consumedKeys(state.input) == [];
    },
  );
});
