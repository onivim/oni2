open Oni_Model;
open Oni_IntegrationTestLib;

runTest(~name="AddRemoveSplitTest", (dispatch, wait, _runEffects) => {
  wait(~name="Wait for split to be created 1", (state: State.t) => {
    let splitCount =
      state.layout |> Feature_Layout.visibleEditors |> List.length;
    splitCount == 1;
  });

  let initialEditorId: ref(option(int)) = ref(None);

  wait(~name="Wait for initial editor", (state: State.t) => {
    initialEditorId :=
      Some(
        Feature_Layout.activeEditor(state.layout)
        |> Feature_Editor.Editor.getId,
      );

    true;
  });

  runCommand(
    ~dispatch,
    Feature_Layout.Commands.splitVertical
    |> Core.Command.map(msg => Model.Actions.Layout(msg)),
  );

  wait(
    ~name="Wait for split to be created, and editor to be focused",
    (state: State.t) => {
    let splitCount =
      state.layout |> Feature_Layout.visibleEditors |> List.length;

    let editorId =
      state.layout
      |> Feature_Layout.activeEditor
      |> Feature_Editor.Editor.getId;

    splitCount == 2 && Some(editorId) != initialEditorId^;
  });

  dispatch(QuitBuffer(Vim.Buffer.getCurrent(), false));

  wait(~name="Wait for split to be closed", (state: State.t) => {
    let splitCount =
      state.layout |> Feature_Layout.visibleEditors |> List.length;

    prerr_endline("Split count: " ++ string_of_int(splitCount));

    splitCount == 1;
  });
});
