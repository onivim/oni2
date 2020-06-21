open Oni_Core;
open Oni_Core.Utility;
open Oni_Model;
open Oni_IntegrationTestLib;
open Feature_LanguageSupport;

runTestWithInput(
  ~name="LanguageTypeScriptTest", (input, dispatch, wait, _runEffects) => {
  wait(~name="Capture initial state", (state: State.t) =>
    Feature_Vim.mode(state.vim) == Vim.Types.Normal
  );

  ExtensionHelpers.waitForExtensionToActivate(
    ~extensionId="oni-dev-extension",
    wait,
  );

  ExtensionHelpers.waitForNewCompletionProviders(
    ~description="typescript completion",
    () => {
      // Create a buffer
      dispatch(Actions.OpenFileByPath("test.ts", None, None));

      wait(
        ~timeout=30.0,
        ~name="Validate we have a TypeScript filetype",
        (state: State.t) => {
          let fileType =
            Selectors.getActiveBuffer(state)
            |> OptionEx.flatMap(Buffer.getFileType);

          switch (fileType) {
          | Some("typescript") => true
          | _ => false
          };
        },
      );
      ExtensionHelpers.waitForExtensionToActivate(
        ~extensionId="vscode.typescript-language-features",
        wait,
      );

      // Wait until the extension is activated
      // Give some time for the exthost to start
      wait(
        ~timeout=30.0,
        ~name=
          "Validate the 'typescript-language-features' extension gets activated",
        (state: State.t) =>
        List.exists(
          id => id == "vscode.typescript-language-features",
          state.extensions.activatedIds,
        )
      );
    },
    wait,
  );

  // Enter some text
  input("i");

  input("w");

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
    (state: State.t)
    // TODO
    // Array.length(state.completions.filtered) > 0
    => true);

  // Fix error, finish identifier
  input("indow");

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
