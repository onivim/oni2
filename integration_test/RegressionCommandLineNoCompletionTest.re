open Oni_Model;
open Oni_IntegrationTestLib;

runTest(~name="Regression: Command line no completions", (dispatch, wait, _) => {
  wait(~name="Initial mode is normal", (state: State.t) =>
    state.vimMode == Vim.Types.Normal
  );

  dispatch(KeyboardInput(":"));

  wait(~name="Mode switches to command line", (state: State.t) =>
    state.vimMode == Vim.Types.CommandLine
  );

  dispatch(KeyboardInput("e"));
  wait(~name="Mode switches to command line", (state: State.t) =>
    switch (state.quickmenu) {
    | Some(quickmenu) => quickmenu.query == "e"
    | None => false
    }
  );

  dispatch(KeyboardInput("h"));

  wait(~name="Mode switches to command line", (state: State.t) =>
    switch (state.quickmenu) {
    | Some(quickmenu) => quickmenu.query == "eh"
    | None => false
    }
  );
});
