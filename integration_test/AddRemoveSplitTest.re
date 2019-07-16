open Oni_Model;
open Oni_IntegrationTestLib;

runTest(~name="AddRemoveSplitTest", (dispatch, wait) => {
  wait(~name="Wait for split to be created 1", (state: State.t) => {
    let splitCount =
      state.windowManager.windowTree |> WindowTree.getSplits |> List.length;
      splitCount == 1;
  });

  dispatch(Command("view.splitVertical"));

  wait(~name="Wait for split to be created", (state: State.t) => {
    let splitCount =
      state.windowManager.windowTree |> WindowTree.getSplits |> List.length;

    splitCount == 2;
  });

  dispatch(QuitBuffer(Vim.Buffer.getCurrent(), false));

  wait(~name="Wait for split to be closed", (state: State.t) => {
    let splitCount =
      state.windowManager.windowTree |> WindowTree.getSplits |> List.length;

    splitCount == 1;
  });
});
