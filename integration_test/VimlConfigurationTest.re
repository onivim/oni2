open Oni_Model;
open Oni_IntegrationTestLib;

let configuration = Some({|
"experimental.viml": "nnoremap ; :"
|});

runTest(~configuration, ~name="Regression: Command line no completions", (dispatch, wait, _) => {
  wait(~name="Initial mode is normal", (state: State.t) =>
    state.mode == Vim.Types.Normal
  );

  dispatch(KeyboardInput(";"));

  wait(~name="Mode switches to command line", (state: State.t) =>
    state.mode == Vim.Types.CommandLine
  );

  dispatch(KeyboardInput("e"));
  wait(~name="Mode switches to command line", (state: State.t) =>
    state.commandline.text == "e"
  );

  dispatch(KeyboardInput("h"));

  wait(~name="Mode switches to command line", (state: State.t) =>
    state.commandline.text == "eh"
  );
});
