open Oni_Core;
open Oni_Model;
open Oni_IntegrationTestLib;

// Validate that textmate highlight runs
runTest(~name="RegressionDeleteAllLinesTest", (dispatch, wait, runEffects) => {
  wait(~name="Capture initial state", (state: State.t) =>
    state.vimMode == Vim.Types.Normal
  );

  let testFile = getAssetPath("some-test-file.txt");

  // Create a buffer
  dispatch(Actions.OpenFileByPath(testFile, None, None));

  // Wait for lines to be available in buffer
  wait(~name="Buffer is loaded", (state: State.t) =>
    switch (Selectors.getActiveBuffer(state)) {
    | None => false
    | Some(buf) => Buffer.getNumberOfLines(buf) > 1
    }
  );

  wait(
    ~name="Dispatch gotOriginalContent to simulate a diff", (state: State.t) => {
    switch (Selectors.getActiveBuffer(state)) {
    | None => failwith("Should have an active buffer now!")
    | Some(buf) =>
      let bufferId = Buffer.getId(buf);
      let lines = Buffer.getLines(buf);
      dispatch(Actions.GotOriginalContent({bufferId, lines}));
      true;
    }
  });

  // Delete all contents of the buffer
  dispatch(KeyboardInput("g"));
  dispatch(KeyboardInput("g"));

  dispatch(KeyboardInput("d"));

  dispatch(KeyboardInput("G"));
  runEffects();

  // Wait for highlights to show up
  wait(~name="Verify all lines are deleted", ~timeout=10.0, (state: State.t) => {
    switch (Selectors.getActiveBuffer(state)) {
    | None => false
    | Some(buf) => Buffer.getNumberOfLines(buf) == 0
    }
  });
});
