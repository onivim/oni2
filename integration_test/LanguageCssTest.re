open Oni_Core;
open Oni_Model;
open Oni_IntegrationTestLib;

// This test validates:
// - The 'oni-dev' extension gets activated
// - When typing in an 'oni-dev' buffer, we get some completion results
runTest(~name="LanguageCssTest", ({input, dispatch, wait, _}) => {
  wait(~name="Capture initial state", (state: State.t) =>
    Selectors.mode(state) |> Vim.Mode.isNormal
  );

  ExtensionHelpers.waitForExtensionToActivate(
    ~extensionId="oni-dev-extension",
    wait,
  );

  ExtensionHelpers.waitForNewCompletionProviders(
    ~description="css completion",
    () => {
      // Create a buffer
      dispatch(
        Actions.OpenFileByPath("test.css", SplitDirection.Current, None),
      );

      // Wait for the CSS filetype
      wait(
        ~timeout=30.0,
        ~name="Validate we have a CSS filetype",
        (state: State.t) => {
          let fileType =
            Selectors.getActiveBuffer(state)
            |> Option.map(Buffer.getFileType)
            |> Option.map(Buffer.FileType.toString);

          fileType == Some("css");
        },
      );

      ExtensionHelpers.waitForExtensionToActivate(
        ~extensionId="vscode.css-language-features",
        wait,
      );
    },
    wait,
  );

  // Enter some text
  input("i");

  input("a");

  // Should get an error diagnostic
  wait(
    ~timeout=30.0,
    ~name=
      "Validate a diagnostic showed up, since our current input is erroneous",
    (state: State.t) => {
      let bufferOpt = Selectors.getActiveBuffer(state);

      switch (bufferOpt) {
      | Some(buffer) =>
        let diags =
          Feature_Diagnostics.getDiagnostics(state.diagnostics, buffer);
        List.length(diags) > 0;
      | _ => false
      };
    },
  );

  // Should've also gotten some completions...
  wait(
    ~timeout=30.0,
    ~name="Validate we also got some completions",
    (state: State.t) =>
    state.languageSupport
    |> Feature_LanguageSupport.Completion.availableCompletionCount > 0
  );

  // Finish input, clear diagnostics
  input(" ");
  input("{");
  input("color:red");
  input("}");

  wait(
    ~timeout=30.0,
    ~name="Validate no diagnostics now",
    (state: State.t) => {
      let bufferOpt = Selectors.getActiveBuffer(state);

      switch (bufferOpt) {
      | Some(buffer) =>
        let diags =
          Feature_Diagnostics.getDiagnostics(state.diagnostics, buffer);
        List.length(diags) == 0;
      | _ => false
      };
    },
  );
});
