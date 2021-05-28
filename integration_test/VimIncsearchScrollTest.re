open Oni_Core;
open Oni_Model;
open Oni_IntegrationTestLib;

runTest(~name="VimIncsearchScrollTest", ({dispatch, wait, input, key, _}) => {
  wait(~name="Initial mode is normal", (state: State.t) =>
    Selectors.mode(state) |> Vim.Mode.isNormal
  );

  let largeCFile = getAssetPath("large-c-file.c");
  dispatch(Actions.OpenFileByPath(largeCFile, SplitDirection.Current, None));

  // Wait for file to load
  wait(
    ~timeout=30.0,
    ~name="Validate buffer is loaded",
    (state: State.t) => {
      let fileType =
        Selectors.getActiveBuffer(state)
        |> Option.map(Buffer.getFileType)
        |> Option.map(Buffer.FileType.toString);

      fileType == Some("c");
    },
  );

  // Start searching in the file:
  input("/");

  // Validate we're now in command line
  wait(~name="Mode switches to command line", (state: State.t) => {
    Vim.Mode.isCommandLine(Selectors.mode(state))
  });

  // Search for something far down, that should trigger a scroll
  // "xor" is on line 983...
  input("\"xor");

  // Validate editor has scrolled
  wait(~name="Validate editor has scrolled some distance", (state: State.t) => {
    let scrollY =
      state.layout
      |> Feature_Layout.activeEditor
      |> Feature_Editor.Editor.scrollY;

    scrollY > 100.;
  });

  // Cancel search, we should scroll back up...
  key(EditorInput.Key.Escape);

  // Validate editor has scrolled back to top
  wait(
    ~name="Validate editor is back to start",
    ~timeout=30.,
    (state: State.t) => {
      let scrollY =
        state.layout
        |> Feature_Layout.activeEditor
        |> Feature_Editor.Editor.scrollY;

      scrollY < 1.;
    },
  );
});
