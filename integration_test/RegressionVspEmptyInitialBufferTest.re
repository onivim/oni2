// Regression test for #594:
// When running ':vsp' on an initial buffer, we shouldn't get a new buffer

// This is sort of an indirect way to test #594, but one of the contributing
// factors to the crash is that we were deciding to open a new / invalid
// buffer - so by testing that we aren't opening a new buffer at all,
// we can avoid that particular issue.

open Oni_Model;
open Oni_IntegrationTestLib;

runTest(~name="RegressionVspEmpty", (_, wait, _) => {
  wait(~name="Wait for split to be created 1", (state: State.t) => {
    let splitCount =
      state.layout.windowTree
      |> Feature_Layout.WindowTree.getSplits
      |> List.length;
    splitCount == 1;
  });

  /* :vsp with no arguments should create a second split w/ same buffer */
  Vim.command("vsp");

  wait(~name="Wait for split to be created", (state: State.t) => {
    let splitCount =
      state.layout.windowTree
      |> Feature_Layout.WindowTree.getSplits
      |> List.length;

    splitCount == 2;
  });

  /* Validate the editors all have same buffer id */
  wait(~name="Wait for split to be created", (state: State.t) => {
    let splits = Feature_Layout.WindowTree.getSplits(state.layout.windowTree);

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
