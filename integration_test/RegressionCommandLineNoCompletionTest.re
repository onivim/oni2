open Oni_Model;
open Oni_IntegrationTestLib;

runTest(~name="Regression: Command line no completions", (dispatch, wait, _) => {
  wait(~name="Initial mode is normal", (state: State.t) =>
    state.mode == Vim.Types.Normal
  );

  dispatch(KeyboardInput(":"));

  wait(~name="Mode switches to command line", (state: State.t) =>
    state.mode == Vim.Types.CommandLine
  );

  dispatch(KeyboardInput("e"));
  wait(~name="Mode switches to command line", (state: State.t) =>
    switch (state.menu) {
    | Some(menuState) => menuState.text == "e"
    | None => false
    }
  );

  dispatch(KeyboardInput("h"));

  wait(~name="Mode switches to command line", (state: State.t) =>
    switch (state.menu) {
    | Some(menuState) => menuState.text == "eh"
    | None => false
    }
  );
});
