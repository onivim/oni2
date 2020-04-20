open Oni_Model;
open Oni_IntegrationTestLib;

runTest(~name="RegressionVspEmpty", (_, wait, _) => {
  wait(~name="Wait for split to be created 1", (state: State.t) => {
    let splitCount = state.layout |> Feature_Layout.windows |> List.length;
    splitCount == 1;
  });

  Vim.command("e test.txt");

  /* :vsp with no arguments should create a second split w/ same buffer */
  Vim.command("vsp");

  wait(~name="Wait for split to be created", (state: State.t) => {
    let splitCount = state.layout |> Feature_Layout.windows |> List.length;

    splitCount == 2;
  });

  /* Validate the editors all have same buffer id */
  wait(~name="Wait for split to be created", (state: State.t) => {
    let splits = Feature_Layout.windows(state.layout);

    let firstSplit = List.nth(splits, 0);
    let secondSplit = List.nth(splits, 1);

    let firstActiveEditor =
      Selectors.getEditorGroupById(state, firstSplit)
      |> Selectors.getActiveEditor;

    let secondActiveEditor =
      Selectors.getEditorGroupById(state, secondSplit)
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
