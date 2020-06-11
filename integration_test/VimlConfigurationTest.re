open Oni_Model;
open Oni_IntegrationTestLib;

let configuration = Some({|
{ "experimental.viml": "nnoremap ; :" }
|});

runTest(
  ~configuration,
  ~name="Viml Configuration Block",
  (dispatch, wait, _) => {
    wait(~name="Initial mode is normal", (state: State.t) =>
      Feature_Vim.mode(state.vim) == Vim.Types.Normal
    );

    // Because of our 'experimental.viml' block, the ';' semicolon
    // is mapped to ':' - so sending it should open the command line.
    dispatch(KeyboardInput(";"));

    wait(~name="Mode switches to command line", (state: State.t) =>
      Feature_Vim.mode(state.vim) == Vim.Types.CommandLine
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
  },
);
