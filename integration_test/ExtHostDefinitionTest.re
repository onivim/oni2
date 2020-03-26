open Oni_Core;
open Oni_Core.Utility;
open Oni_Model;
open Oni_IntegrationTestLib;
open Feature_LanguageSupport;
open Feature_Editor;

// This test validates:
// - The 'oni-dev' extension gets activated
// - We get a definition response
runTestWithInput(
  ~name="ExtHostDefinitionTest", (input, _dispatch, wait, _runEffects) => {
  wait(~name="Capture initial state", (state: State.t) =>
    state.vimMode == Vim.Types.Normal
  );

  // Wait until the extension is activated
  // Give some time for the exthost to start
  wait(
    ~timeout=30.0,
    ~name="Validate the 'oni-dev' extension gets activated",
    (state: State.t) =>
    List.exists(
      id => id == "oni-dev-extension",
      state.extensions.activatedIds,
    )
  );

  // Create a buffer
  Vim.command("new test.oni-dev");

  // Wait for the oni-dev filetype
  wait(
    ~timeout=30.0,
    ~name="Wait for oni-dev filetype to show up",
    (state: State.t) => {
      let fileType =
        Selectors.getActiveBuffer(state)
        |> OptionEx.flatMap(Buffer.getFileType);

      switch (fileType) {
      | Some("oni-dev") => true
      | _ => false
      };
    },
  );

  // Enter some text
  input("i");

  input("a");
  input("b");
  input("c");

  // Workaround a bug where cursor position is offset with <esc>
  input("<esc>");
  input("h");

  // Should get a definition
  wait(
    ~timeout=30.0,
    ~name="Validate we get some completions from the 'oni-dev' extension",
    (state: State.t) => {
      let maybeBuffer = Selectors.getActiveBuffer(state);

      let maybeEditor =
        state
        |> Selectors.getActiveEditorGroup
        |> Selectors.getActiveEditor
        |> Option.map(editor => Editor.getPrimaryCursor(editor));

      let isDefinitionAvailable = (buffer, location) => {
        Definition.isAvailable(
          Buffer.getId(buffer),
          location,
          state.definition,
        );
      };

      OptionEx.map2(isDefinitionAvailable, maybeBuffer, maybeEditor)
      |> Option.value(~default=false);
    },
  );
});
