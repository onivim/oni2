open Oni_Model;
open Oni_IntegrationTestLib;

runTest(~name="InsertMode test", (dispatch, wait) => {
  wait(~name="Initial mode is normal", (state: State.t) =>
    state.mode == Vim.Types.Normal
  );

  Vim.command("vsp");

  wait(~name="Wait for split to be created", (state: State.t) => {
    let splitCount = state.windowManager.windowTree
    |> WindowTree.getSplits
    |> List.length

    splitCount == 2
  });
});
