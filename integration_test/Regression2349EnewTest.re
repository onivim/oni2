open Oni_Model;
open Oni_IntegrationTestLib;

runTest(~name="Regression: Command line no completions", (dispatch, wait, _) => {
  wait(~name="Initial mode is normal", (state: State.t) =>
    Feature_Vim.mode(state.vim) == Vim.Mode.Normal
  );

  let initialEditorId = ref(None);

  wait(~name="Wait for first editor to be created", (state: State.t) => {
    let initialEditor = Feature_Layout.activeEditor(state.layout);
    initialEditorId := Some(initialEditor |> Feature_Editor.Editor.getId);
    true;
  });

  dispatch(Actions.VimExecuteCommand("enew"));

  wait(~name="New editor created", (state: State.t) => {
    let currentEditorId =
      state.layout
      |> Feature_Layout.activeEditor
      |> Feature_Editor.Editor.getId;

    Some(currentEditorId) != initialEditorId^;
  });

  wait(~name="New buffer should have filetype plaintext", (state: State.t) => {
    let currentBufferId =
      state.layout
      |> Feature_Layout.activeEditor
      |> Feature_Editor.Editor.getBufferId;

    let fileTypeString =
      state.buffers
      |> Feature_Buffers.get(currentBufferId)
      |> Option.map(Oni_Core.Buffer.getFileType)
      |> Option.map(Oni_Core.Buffer.FileType.toString);

    fileTypeString == Some("plaintext");
  });
});
