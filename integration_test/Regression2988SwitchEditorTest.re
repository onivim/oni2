open Oni_Model;
open Oni_IntegrationTestLib;

runTest(~name="#2988: SwitchEditorTest", ({dispatch, wait, runEffects, _}) => {
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

  // Open a couple files
  dispatch(VimExecuteCommand({allowAnimation: true, command: "e a.txt"}));
  runEffects();
  dispatch(VimExecuteCommand({allowAnimation: true, command: "e b.txt"}));
  runEffects();

  let initialEditorCount = ref(0);

  wait(~name="Wait for editor tabs to show up in UI", (state: State.t) => {
    let editorCount =
      state.layout |> Feature_Layout.activeGroupEditors |> List.length;

    initialEditorCount := editorCount;
    editorCount == 3;
  });

  dispatch(VimExecuteCommand({allowAnimation: true, command: "b 2"}));
  runEffects();

  wait(
    ~name="Check number of editors after switching buffer back to 2",
    (state: State.t) => {
    let editorCount =
      state.layout |> Feature_Layout.activeGroupEditors |> List.length;

    // Should not open any new editor tabs...
    editorCount == initialEditorCount^;
  });

  dispatch(VimExecuteCommand({allowAnimation: true, command: "b 3"}));
  runEffects();

  wait(
    ~name="Check number of editors after switching buffer back to 3",
    (state: State.t) => {
    let editorCount =
      state.layout |> Feature_Layout.activeGroupEditors |> List.length;

    // Should not open any new editor tabs...
    editorCount == initialEditorCount^;
  });
});
