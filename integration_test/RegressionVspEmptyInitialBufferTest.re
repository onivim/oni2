// Regression test for #594:
// When running ':vsp' on an initial buffer, we shouldn't get a new buffer

// This is sort of an indirect way to test #594, but one of the contributing
// factors to the crash is that we were deciding to open a new / invalid
// buffer - so by testing that we aren't opening a new buffer at all,
// we can avoid that particular issue.

open Oni_Core;
open Oni_Model;
open Oni_IntegrationTestLib;
module Editor = Feature_Editor.Editor;

runTest(~name="RegressionVspEmpty", ({wait, _}) => {
  wait(~name="Wait for split to be created 1", (state: State.t) => {
    let splitCount =
      state.layout |> Feature_Layout.visibleEditors |> List.length;
    splitCount == 1;
  });

  // Wait for initial buffer
  wait(~name="Initial buffer", state => {
    let maybeFilePath =
      state |> Selectors.getActiveBuffer |> Option.map(Buffer.getFilePath);

    maybeFilePath != None;
  });

  /* :vsp with no arguments should create a second split w/ same buffer */
  ignore(Vim.command("vsp"): (Vim.Context.t, list(Vim.Effect.t)));

  wait(~name="Wait for split to be created", (state: State.t) => {
    let splitCount =
      state.layout |> Feature_Layout.visibleEditors |> List.length;

    splitCount == 2;
  });

  /* Validate the editors all have same buffer id */
  wait(~name="Wait for split to be created", (state: State.t) => {
    switch (Feature_Layout.visibleEditors(state.layout)) {
    | [e1, e2] =>
      let bufferId1 = Editor.getBufferId(e1);
      let bufferId2 = Editor.getBufferId(e2);
      print_endline("e1 buffer id: " ++ string_of_int(bufferId1));
      print_endline("e2 buffer id: " ++ string_of_int(bufferId2));
      bufferId1 == bufferId2;
    | _ => false
    }
  });
});
