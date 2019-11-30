open Oni_Core.Utility;
open Oni_Model;
open Oni_IntegrationTestLib;

// This test validates:
// - The 'oni-dev' extension gets activated
// - When typing in an 'oni-dev' buffer, the buffer received by the extension host
// is in sync with the buffer in the main process
runTestWithInput(
  ~name="ExtHostBufferUpdates", (input, dispatch, wait, _runEffects) => {
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
  input("i");

  input("a");
  input("<CR>");

  input("b");
  input("<CR>");

  input("c");
  input("<esc>");

  // Helper function to wait for the buffer text to match
  // The extension maps newlines -> |
  let waitForTextToMatch = expectedText =>
    wait(
      ~timeout=30.0,
      ~name="Validate buffer text matches: " ++ expectedText,
      (state: State.t) => {
        dispatch(
          Actions.CommandExecuteContributed("developer.oni.getBufferText"),
        );
        switch (state.notifications) {
        | [hd, ..._] => String.equal(hd.message, "fulltext:" ++ expectedText)
        | [] => false
        };
      },
    );

  // TODO: Do we need to wait to ensure the buffer update gets sent?
  waitForTextToMatch("a|b|c");

  // Now, delete a line - we had some bugs where deletes were not sync'd properly
  input("gg");
  input("j");
  input("dd");

  waitForTextToMatch("a|c");

  // Undo the change - we also had bugs here!
  input("u");

  waitForTextToMatch("a|b|c");

  // Finally, modify a single line
  input("gg");
  input("Iabc");

  waitForTextToMatch("abca|b|c");
});
