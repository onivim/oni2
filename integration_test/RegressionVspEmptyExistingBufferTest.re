open Oni_Model;
open Oni_IntegrationTestLib;

runTest(~name="RegressionVspEmpty", (_, wait, _) => {
  wait(~name="Wait for split to be created 1", (state: State.t) => {
    let splitCount =
      state.windowManager.windowTree |> WindowTree.getSplits |> List.length;
    splitCount == 1;
  });

  Vim.command("e test.txt") |> ignore;

  /* :vsp with no arguments should create a second split w/ same buffer */
  Vim.command("vsp") |> ignore;

  wait(~name="Wait for split to be created", (state: State.t) => {
    let splitCount =
      state.windowManager.windowTree |> WindowTree.getSplits |> List.length;

    splitCount == 2;
  });

  /* Validate the editors all have same buffer id */
  wait(~name="Wait for split to be created", (state: State.t) => {
    let splits = WindowTree.getSplits(state.windowManager.windowTree);

    let firstSplit = List.nth(splits, 0);
    let secondSplit = List.nth(splits, 1);

    let firstActiveEditor =
      Selectors.getEditorGroupById(state, firstSplit.editorGroupId)
      |> Selectors.getActiveEditor;

    let secondActiveEditor =
      Selectors.getEditorGroupById(state, secondSplit.editorGroupId)
      |> Selectors.getActiveEditor;

    switch (firstActiveEditor, secondActiveEditor) {
    | (Some(e1), Some(e2)) =>
      print_endline("e1 buffer id: " ++ string_of_int(e1.bufferId));
      print_endline("e2 buffer id: " ++ string_of_int(e2.bufferId));
      e1.bufferId == e2.bufferId;
    | _ => false
    };
  });
});
