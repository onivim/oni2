open Oni_Model;
open Oni_IntegrationTestLib;

// Regression test case covering #733:
// Closing all active splits causes no other splits to be opened
runTestWithInput(
  ~name="Regression733Test", (input, dispatch, wait, _runEffects) => {
  wait(~name="Initial mode is normal", (state: State.t) =>
    Feature_Vim.mode(state.vim) == Vim.Types.Normal
  );

  let activeEditorRef = ref(None);

  wait(~name="Verify there is an editor open...", (state: State.t) => {
    state
    |> Selectors.getActiveEditorGroup
    |> Option.map(EditorGroup.count)
    |> Option.map(count => count == 1)
    |> Option.value(~default=false)
  });

  wait(~name="Wait for active editor", (state: State.t) => {
    let maybeActiveEditor =
      state |> Selectors.getActiveEditorGroup |> Selectors.getActiveEditor;

    activeEditorRef := maybeActiveEditor;
    maybeActiveEditor != None;
  });

  // Modify the file
  input("o");
  input("a");
  input("b");
  input("c");

  switch (activeEditorRef^) {
  | None => failwith("We should've had an editor now")
  | Some(editor) =>
    dispatch(ViewCloseEditor(Feature_Editor.Editor.getId(editor)))
  };

  wait(~name="Verify there are no editors open...", (state: State.t) => {
    state
    |> Selectors.getActiveEditorGroup
    |> Option.map(EditorGroup.count)
    |> Option.map(count => count == 0)
    |> Option.value(~default=false)
  });

  dispatch(Actions.OpenFileByPath("regression-733.txt", None, None));

  wait(~name="...but one reopens when a file is opened", (state: State.t) => {
    state
    |> Selectors.getActiveEditorGroup
    |> Option.map(EditorGroup.count)
    |> Option.map(count => count == 1)
    |> Option.value(~default=false)
  });
});
