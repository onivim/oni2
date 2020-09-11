open Oni_Model;
open Oni_IntegrationTestLib;

runTest(~name="Regression: Command line no completions", (dispatch, wait, _) => {
  wait(~name="Initial mode is normal", (state: State.t) =>
    Feature_Vim.mode(state.vim) == Vim.Mode.Normal
  );

  dispatch(KeyboardInput({isText: true, input: ":"}));

  wait(~name="Mode switches to command line", (state: State.t) =>
    Feature_Vim.mode(state.vim) == Vim.Mode.CommandLine
  );

  dispatch(KeyboardInput({isText: true, input: "e"}));
  wait(~name="Mode switches to command line", (state: State.t) =>
    switch (state.quickmenu) {
    | Some(quickmenu) =>
      quickmenu.inputText |> Component_InputText.value == "e"
    | None => false
    }
  );

  dispatch(KeyboardInput({isText: true, input: "h"}));

  wait(~name="Mode switches to command line", (state: State.t) =>
    switch (state.quickmenu) {
    | Some(quickmenu) =>
      quickmenu.inputText |> Component_InputText.value == "eh"
    | None => false
    }
  );
});
