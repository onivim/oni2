// Regression test for #3150:
// When running ':tabnew'/':tabedit' on a buffer, only a single
// editor tab should be visible.

open Oni_Core;
open Oni_Model;
open Oni_IntegrationTestLib;
module Editor = Feature_Editor.Editor;

runTest(~name="RegressionLayoutTabSingleEditor", ({wait, _}) => {
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

  wait(
    ~name="Prior to new tab, there should be a single layout",
    (state: State.t) => {
    let layoutCount = state.layout |> Feature_Layout.layouts |> List.length;

    layoutCount == 1;
  });

  /* :vsp with no arguments should create a second split w/ same buffer */
  ignore(Vim.command("tabnew"): (Vim.Context.t, list(Vim.Effect.t)));

  wait(~name="Wait for group to be created", (state: State.t) => {
    let layoutCount = state.layout |> Feature_Layout.layouts |> List.length;

    layoutCount == 2;
  });

  wait(
    ~name="There should only be a single editor in the new tab",
    (state: State.t) => {
    let group = state.layout |> Feature_Layout.activeGroup;

    let editorCount = group |> Feature_Layout.Group.allEditors |> List.length;

    editorCount == 1;
  });
});
