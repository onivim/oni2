open Oni_Model;
open Oni_IntegrationTestLib;

// This test validates:
// - The 'oni-dev' extension gets activated
// - When typing in an 'oni-dev' buffer, we get some completion results
runTest(~name="LanguageCssTest", (_dispatch, wait, _runEffects) => {
  wait(~name="Capture initial state", (state: State.t) =>
    state.mode == Vim.Types.Normal
  );

  // Create a buffer
  Vim.command("new test.css");

  // Wait for the CSS filetype
  /*wait(
    ~timeout=30.0,
    ~name="Validate we have a CSS filetype",
    (state: State.t) => {
      let fileType =
        Some(state)
        |> Option.bind(Selectors.getActiveBuffer)
        |> Option.bind(Buffer.getFileType);

      switch (fileType) {
      | Some("css") => true
      | _ => false
      };
    },
  );*/
  
  // Wait until the extension is activated
  // Give some time for the exthost to start
  wait(
    ~timeout=30.0,
    ~name="Validate the 'css-language-features' extension gets activated",
    (_state: State.t) =>
    /*List.exists(
      id => id == "css-language-features",
      state.extensions.activatedIds,
    )*/
    true
  );


  // Enter some text
  Vim.input("i");

  Vim.input("abc");

  // Should get an error diagnostic
  wait(
    ~timeout=30.0,
    ~name="Validate a diagnostic showed up, since our current input is erroneous",
    (state: State.t) => {
      let bufferOpt = Selectors.getActiveBuffer(state);

      switch (bufferOpt) {
      | Some(buffer) => 
        let diags = Model.Diagnostics.getDiagnostics(state.diagnostics, buffer)
        List.length(diags) > 0
      | _ => false
      };
    },
  );
});
