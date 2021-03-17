open Oni_Core;
open Oni_Model;
open Oni_IntegrationTestLib;

// Regression test case covering #600:
// https://github.com/onivim/oni2/issues/600
//
// Verify some simple cases around the file modified flag
runTest(
  ~name="RegressionFileModifiedIndication",
  ({input, dispatch, wait, runEffects, _}) => {
  wait(~name="Initial mode is normal", (state: State.t) =>
    Selectors.mode(state) |> Vim.Mode.isNormal
  );

  let initialBuffer = Vim.Buffer.getCurrent();

  dispatch(
    Actions.OpenFileByPath(
      "regression-file-modified-indication.txt",
      SplitDirection.Current,
      None,
    ),
  );
  runEffects();

  wait(~name="Wait for new buffer", (state: State.t) => {
    let activeBufferId =
      state
      |> Selectors.getActiveBuffer
      |> Option.map(Buffer.getId)
      |> Option.value(~default=-1);

    activeBufferId != Vim.Buffer.getId(initialBuffer);
  });

  let id = Vim.Buffer.getCurrent() |> Vim.Buffer.getId;

  let _ = input("o");
  let _ = input("a");

  let validateBufferCondition = (f, state: State.t) => {
    let buffer = Selectors.getBufferById(state, id);
    switch (buffer) {
    | Some(v) => f(v)
    | None => false
    };
  };

  wait(~name="Wait for modified flag to be set", (state: State.t) =>
    validateBufferCondition(b => Buffer.isModified(b) == true, state)
  );

  // Save file, should clear modified flag
  let _ = input("<esc>");
  Vim.command("w") |> ignore;

  wait(~name="Wait for modified flag to be set", (state: State.t) =>
    validateBufferCondition(b => Buffer.isModified(b) == false, state)
  );

  // Make another modification, should set it again
  let _ = input("o");
  let _ = input("a");

  wait(~name="Wait for modified flag to be set", (state: State.t) =>
    validateBufferCondition(b => Buffer.isModified(b) == true, state)
  );
});
