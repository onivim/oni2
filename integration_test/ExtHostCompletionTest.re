open Oni_Core.Utility;
open Oni_Model;
open Oni_IntegrationTestLib;

// This test validates:
// - The 'oni-dev' extension gets activated
// - When typing in an 'oni-dev' buffer, we get some completion results
runTest(~name="ExtHostCompletionTest", (_dispatch, wait, _runEffects) => {
  wait(~name="Capture initial state", (state: State.t) =>
    state.mode == Vim.Types.Normal
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
    ~name="Validate the 'oni-dev' extension gets activated",
    (state: State.t) => {
      let fileType =
        Some(state)
        |> Option.bind(Selectors.getActiveBuffer)
        |> Option.bind(Buffer.getFileType);

      switch (fileType) {
      | Some("oni-dev") => true
      | _ => false
      };
    },
  );

  // Enter some text
  let _ = Vim.input("i");

  let _ = Vim.input("H");

  // Should get completions
  wait(
    ~timeout=30.0,
    ~name="Validate the 'oni-dev' extension gets activated",
    (state: State.t) => {
    List.length(state.completions.filteredCompletions) > 0
  });
});
