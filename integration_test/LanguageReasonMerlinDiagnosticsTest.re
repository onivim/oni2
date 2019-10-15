open Oni_Model;
open Oni_IntegrationTestLib;

let configuration = Some({|
{ "experimental.merlin": true }
|});

// Regression test case covering #600:
// https://github.com/onivim/oni2/issues/600
//
// Verify some simple cases around the file modified flag
runTest(
  ~configuration,
  ~name="LanguageReasonMerlinDiagnosticsTest",
  (dispatch, wait, runEffects) => {
    wait(~name="Initial mode is normal", (state: State.t) =>
      state.mode == Vim.Types.Normal
    );

    let buf = Vim.Buffer.openFile("testcase.re");
    Vim.Buffer.setCurrent(buf);

    let id = Vim.Buffer.getCurrent() |> Vim.Buffer.getId;

    wait(
      ~timeout=30., ~name="Wait for active buffer to be set", (state: State.t) => {
      switch (Selectors.getActiveBuffer(state)) {
      | None => false
      | Some(v) => Buffer.getId(v) == id
      }
    });

    dispatch(KeyboardInput("o"));
    dispatch(KeyboardInput("1"));
    dispatch(KeyboardInput("."));
    dispatch(KeyboardInput("+"));

    runEffects();

    // At this point, we'd have:
    // 1.+
    // in the buffer - which would be an error (need '+.' in the buffer)

    wait(
      ~timeout=30.,
      ~name="Wait for diagnostic to show up",
      (state: State.t) => {
        let buffer = Buffers.getBuffer(id, state.buffers);
        switch (buffer) {
        | None => false
        | Some(v) =>
          let diagnostics = Diagnostics.getDiagnostics(state.diagnostics, v);
          List.length(diagnostics) > 0;
        };
      },
    );

    Vim.input(".");
    Vim.input("2");
    Vim.input(".");

    // Diagnostics should now go to 0
    wait(
      ~timeout=30.,
      ~name="Wait for diagnostic to be removed",
      (state: State.t) => {
        let buffer = Buffers.getBuffer(id, state.buffers);
        switch (buffer) {
        | None => false
        | Some(v) =>
          let diagnostics = Diagnostics.getDiagnostics(state.diagnostics, v);
          List.length(diagnostics) == 0;
        };
      },
    );
  },
);
