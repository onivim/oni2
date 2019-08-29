open Oni_Model;
open Oni_IntegrationTestLib;

runTest(~name="Regression: Command line no completions", (dispatch, wait, _) => {
  wait(~name="Initial mode is normal", (state: State.t) =>
    state.mode == Vim.Types.Normal
  );

  let createSimpleMenu = (setItems, _, _) => {
    let commands: list(Actions.menuCommand) = [
      {
        category: Some("Preferences"),
        name: "Open configuration file",
        command: () => (),
        icon: None,
      },
    ];

    setItems(commands);
  };

  dispatch(MenuOpen(createSimpleMenu));

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
