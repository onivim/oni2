open Oni_Model;
open Oni_IntegrationTestLib;

runTest(~name="Regression: Command line no completions", (dispatch, wait, _) => {
  wait(~name="Initial mode is normal", (state: State.t) =>
    Feature_Vim.mode(state.vim) == Vim.Types.Normal
  );

  dispatch(KeyboardInput(":"));

  wait(~name="Mode switches to command line", (state: State.t) =>
    Feature_Vim.mode(state.vim) == Vim.Types.CommandLine
  );

  dispatch(KeyboardInput("e"));
  wait(~name="Mode switches to command line", (state: State.t) =>
    switch (state.quickmenu) {
    | Some(quickmenu) => quickmenu.inputText |> Feature_InputText.value == "e"
    | None => false
    }
  );

  dispatch(KeyboardInput("h"));

  wait(~name="Mode switches to command line", (state: State.t) =>
    switch (state.quickmenu) {
    | Some(quickmenu) =>
      quickmenu.inputText |> Feature_InputText.value == "eh"
    | None => false
    }
  );
});
