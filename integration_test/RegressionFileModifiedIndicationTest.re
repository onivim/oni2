open Oni_Model;
open Oni_IntegrationTestLib;

// Regression test case covering #600:
// https://github.com/onivim/oni2/issues/600
//
// Verify some simple cases around the file modified flag
runTest(~name="RegressionFileModifiedIndication", (_, wait, _) => {
  wait(~name="Initial mode is normal", (state: State.t) =>
    state.mode == Vim.Types.Normal
  );

  Vim.command("e regression-file-modified-indication.txt");

  let id = Vim.Buffer.getCurrent() |> Vim.Buffer.getId;

  Vim.input("o");
  Vim.input("a")

  let validateBufferCondition = (f, state: State.t) => {
    let buffer = Selectors.getBufferById(state, id);
    switch (buffer) {
    | Some(v) => f(v)
    | None => false
    }
  };
  
  wait(~name="Wait for modified flag to be set", (state: State.t) => {
    validateBufferCondition((b) => Buffer.isModified(b) == true, state);
  });

  // Save file, should clear modified flag
  Vim.input("<esc>");
  Vim.command("w")

  wait(~name="Wait for modified flag to be set", (state: State.t) => {
    validateBufferCondition((b) => Buffer.isModified(b) == false, state);
  });
  
  // Make another modification, should set it again
  Vim.input("o");
  Vim.input("a")
  
  wait(~name="Wait for modified flag to be set", (state: State.t) => {
    validateBufferCondition((b) => Buffer.isModified(b) == true, state);
  });
});
