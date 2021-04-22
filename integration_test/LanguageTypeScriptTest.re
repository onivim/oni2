open Oni_Core;
open Oni_Model;
open Oni_IntegrationTestLib;

runTest(~name="LanguageTypeScriptTest", ({input, dispatch, wait, _}) => {
  wait(~name="Capture initial state", (state: State.t) =>
    Selectors.mode(state) |> Vim.Mode.isNormal
  );

  ExtensionHelpers.waitForExtensionToActivate(
    ~extensionId="oni-dev-extension",
    wait,
  );

  ExtensionHelpers.waitForNewCompletionProviders(
    ~description="typescript completion",
    () => {
      // Create a buffer
      dispatch(
        Actions.OpenFileByPath("test.ts", SplitDirection.Current, None),
      );

      wait(
        ~timeout=30.0,
        ~name="Validate we have a TypeScript filetype",
        (state: State.t) => {
          let fileType =
            Selectors.getActiveBuffer(state)
            |> Option.map(Buffer.getFileType)
            |> Option.map(Buffer.FileType.toString);

          fileType == Some("typescript");
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
          state.extensions |> Feature_Extensions.activatedIds,
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

  // Fix error, finish identifier
  input("indow");

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
