open Oni_Core;
open Oni_Model;
open Oni_IntegrationTestLib;
open Feature_Editor;

// This test validates:
// - The 'oni-dev' extension gets activated
// - We get a definition response
runTest(~name="ExtHostDefinitionTest", ({input, dispatch, wait, key, _}) => {
  wait(~timeout=30.0, ~name="Exthost is initialized", (state: State.t) =>
    Feature_Exthost.isInitialized(state.exthost)
  );

  // Wait until the extension is activated
  // Give some time for the exthost to start
  wait(
    ~timeout=30.0,
    ~name="Validate the 'oni-dev' extension gets activated",
    (state: State.t) =>
    List.exists(
      id => id == "oni-dev-extension",
      state.extensions |> Feature_Extensions.activatedIds,
    )
  );

  // Create a buffer
  dispatch(
    Actions.OpenFileByPath("test.oni-dev", SplitDirection.Current, None),
  );

  // Wait for the oni-dev filetype
  wait(
    ~timeout=30.0,
    ~name="Wait for oni-dev filetype to show up",
    (state: State.t) => {
      let fileType =
        Selectors.getActiveBuffer(state)
        |> Option.map(Buffer.getFileType)
        |> Option.map(Buffer.FileType.toString);

      fileType == Some("oni-dev");
    },
  );

  // Enter some text
  input("i");

  input("a");
  input("b");
  input("c");

  key(EditorInput.Key.Escape);

  // Should get a definition
  wait(
    ~timeout=30.0,
    ~name="Validate we get some completions from the 'oni-dev' extension",
    (state: State.t) => {
      let editor = Feature_Layout.activeEditor(state.layout);

      Feature_LanguageSupport.Definition.isAvailable(
        ~bufferId=Editor.getBufferId(editor),
        state.languageSupport,
      );
    },
  );
});
