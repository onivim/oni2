open Oni_Core;
open Oni_Core.Utility;
open Oni_Model;
open Oni_IntegrationTestLib;
open Feature_LanguageSupport;

// This test validates:
// - The 'oni-dev' extension gets activated
// - When typing in an 'oni-dev' buffer, we get some completion results
runTestWithInput(
  ~name="LanguageCssTest", (input, dispatch, wait, _runEffects) => {
  wait(~name="Capture initial state", (state: State.t) =>
    state.mode == Vim.Types.Normal
  );

  ExtensionHelpers.waitForExtensionToActivate(
    ~extensionId="oni-dev-extension",
    wait,
  );

  ExtensionHelpers.waitForNewCompletionProviders(
    ~description="css completion",
    () => {
      // Create a buffer
      dispatch(Actions.OpenFileByPath("test.css", None, None));

      // Wait for the CSS filetype
      wait(
        ~timeout=30.0,
        ~name="Validate we have a CSS filetype",
        (state: State.t) => {
          let fileType =
            Selectors.getActiveBuffer(state)
            |> OptionEx.flatMap(Buffer.getFileType);

          switch (fileType) {
          | Some("css") => true
          | _ => false
          };
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
        let diags = Diagnostics.getDiagnostics(state.diagnostics, buffer);
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
    Array.length(state.completions.filtered) > 0
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
        let diags = Diagnostics.getDiagnostics(state.diagnostics, buffer);
        List.length(diags) == 0;
      | _ => false
      };
    },
  );
});
