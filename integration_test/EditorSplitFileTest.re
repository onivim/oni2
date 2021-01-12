open Oni_Model;
open Oni_IntegrationTestLib;

runTest(
  ~name="#2900: Splitting with a file should just show file",
  (dispatch, wait, runEffects) => {
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

  dispatch(
    VimExecuteCommand({allowAnimation: true, command: "vsp some-file.json"}),
  );
  runEffects();

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

  wait(~name="Verify that both groups have a single editor", (state: State.t) => {
    let groups = state.layout |> Feature_Layout.activeLayoutGroups;

    prerr_endline("Groups count: " ++ string_of_int(List.length(groups)));

    groups
    |> List.for_all(group => {
         let editorsInGroup =
           List.length(Feature_Layout.Group.allEditors(group));
         prerr_endline(
           "Editors in group: " ++ string_of_int(editorsInGroup),
         );
         editorsInGroup == 1;
       });
  });
});
