open Oni_Model;
open Oni_IntegrationTestLib;

runTest(~name="AddRemoveSplitTest", (dispatch, wait, _runEffects) => {

  wait(~name="Wait for split to be created 1", (state: State.t) => {
    let splitCount = state.layout |> Feature_Layout.windows |> List.length;
    splitCount == 1;
  });

  let initialEditorId: ref(option(int)) = ref(None);

  wait(~name="Wait for initial editor", (state: State.t) => {
    let maybeEditor = state
    |> Selectors.getActiveEditorGroup
    |> Selectors.getActiveEditor;

    maybeEditor
    |> Option.map(Feature_Editor.Editor.getId)
    |> Option.iter(id => initialEditorId := Some(id));

    maybeEditor != None
  });
  

  dispatch(Command("view.splitVertical"));

  wait(~name="Wait for split to be created, and editor to be focused", (state: State.t) => {
    let splitCount = state.layout |> Feature_Layout.windows |> List.length;

    let maybeEditorId = state
    |> Selectors.getActiveEditorGroup
    |> Selectors.getActiveEditor
    |> Option.map(Feature_Editor.Editor.getId);

    splitCount == 2 && maybeEditorId != None && maybeEditorId != initialEditorId^;
  });

  dispatch(QuitBuffer(Vim.Buffer.getCurrent(), false));

  wait(~name="Wait for split to be closed", (state: State.t) => {
    let splitCount = state.layout |> Feature_Layout.windows |> List.length;

    prerr_endline ("Split count: " ++ string_of_int(splitCount));

    splitCount == 1;
  });
});
