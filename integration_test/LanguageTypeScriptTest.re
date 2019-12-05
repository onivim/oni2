open Oni_Core.Utility;
open Oni_Extensions;
open Oni_Model;
open Oni_IntegrationTestLib;

runTestWithInput(
  ~name="LanguageTypeScriptTest", (input, dispatch, wait, _runEffects) => {
  wait(~name="Capture initial state", (state: State.t) =>
    state.mode == Vim.Types.Normal
  );

  // Create a buffer
  dispatch(Actions.OpenFileByPath("test.ts", None, None));

  wait(
    ~timeout=30.0,
    ~name="Validate we have a TypeScript filetype",
    (state: State.t) => {
      let fileType =
        Some(state)
        |> Option.bind(Selectors.getActiveBuffer)
        |> Option.bind(Buffer.getFileType);

      switch (fileType) {
      | Some("typescript") => true
      | _ => false
      };
    },
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

  // Also, wait for suggest providers to be registered
  wait(
    ~timeout=30.0,
    ~name="Wait for suggest providers for 'typescript' to be registered",
    (state: State.t) =>
    state.languageFeatures
    |> LanguageFeatures.getSuggestProviders("typescript")
    |> (providers => List.length(providers) > 0)
  );

  // Enter some text
  input("i");

  input("w");

  // TODO: Fix diagnostics on Windows!
  if (!Sys.win32) {
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
            Model.Diagnostics.getDiagnostics(state.diagnostics, buffer);
          List.length(diags) > 0;
        | _ => false
        };
      },
    );
  };

  // Should've also gotten some completions...
  wait(
    ~timeout=30.0,
    ~name="Validate we also got some completions",
    (state: State.t) => {
    Model.Completions.getCompletions(state.completions)
    |> (comp => List.length(comp) > 0)
  });

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
          Model.Diagnostics.getDiagnostics(state.diagnostics, buffer);
        List.length(diags) == 0;
      | _ => false
      };
    },
  );
});
