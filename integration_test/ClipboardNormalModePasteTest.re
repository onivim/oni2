/*
 ClipboardNormalModePasteTest

 Test cases for pasting in normal mode:
 - '*' and '+' registers should always paste from clipboard, regardless of configuration
 - named registers (ie, 'a') should _never_ paste from clipboard, regardless of configuration
 - unnamed register should only paste from clipboard if `["paste"]` is set in configuration
 */

open Oni_Model;
open Oni_IntegrationTestLib;

runTest(
  ~name="InsertMode test - effects batched to runEffects",
  (dispatch, wait, runEffects) => {
  wait(~name="Initial mode is normal", (state: State.t) =>
    state.mode == Vim.Types.Normal
  );

  // '*' test case
  setClipboard(Some("abc"));

  dispatch(KeyboardInput("\""));
  dispatch(KeyboardInput("*"));
  dispatch(KeyboardInput("P"));
  runEffects();

  wait(~name="Mode switches to insert", (state: State.t) =>
    switch (Selectors.getActiveBuffer(state)) {
    | None => false
    | Some(buf) =>
      let line = Buffer.getLine(buf, 0);
      Log.info("Current line is: |" ++ line ++ "|");
      String.equal(line, "abc");
    }
  );

  // '+' test case
  setClipboard(Some("def"));

  dispatch(KeyboardInput("\""));
  dispatch(KeyboardInput("*"));
  dispatch(KeyboardInput("P"));
  runEffects();

  wait(~name="Mode switches to insert", (state: State.t) =>
    switch (Selectors.getActiveBuffer(state)) {
    | None => false
    | Some(buf) =>
      let line = Buffer.getLine(buf, 0);
      Log.info("Current line is: |" ++ line ++ "|");
      String.equal(line, "def");
    }
  );

  // 'a' test case
  // yank current line - def - to 'a' register
  dispatch(KeyboardInput("\""));
  dispatch(KeyboardInput("a"));
  dispatch(KeyboardInput("y"));
  dispatch(KeyboardInput("y"));
  runEffects();

  setClipboard(Some("ghi"));

  dispatch(KeyboardInput("\""));
  dispatch(KeyboardInput("a"));
  dispatch(KeyboardInput("P"));
  runEffects();

  wait(~name="Mode switches to insert", (state: State.t) =>
    switch (Selectors.getActiveBuffer(state)) {
    | None => false
    | Some(buf) =>
      let line = Buffer.getLine(buf, 0);
      Log.info("Current line is: |" ++ line ++ "|");
      String.equal(line, "def");
    }
  );

  // Test if the configuration is set - paste from unnamed register will pull from the keyboard
  setClipboard(Some("jkl"));

  wait(~name="Set configuration to pull clipboard on paste", (state: State.t) => {
    let configuration = state.configuration;
    dispatch(
      ConfigurationSet({
        ...configuration,
        default: {
          ...configuration.default,
          vimUseSystemClipboard: {
            yank: false,
            delete: false,
            paste: true,
          },
        },
      }),
    );
    runEffects();
    true;
  });

  dispatch(KeyboardInput("y"));
  dispatch(KeyboardInput("y"));
  dispatch(KeyboardInput("P"));
  runEffects();

  wait(
    ~name=
      "paste from unnamed register pulls from clipboard when ['paste'] is set",
    (state: State.t) =>
    switch (Selectors.getActiveBuffer(state)) {
    | None => false
    | Some(buf) =>
      let line = Buffer.getLine(buf, 0);
      Log.info("Current line is: |" ++ line ++ "|");
      String.equal(line, "jkl");
    }
  );

  // Set configuration back (paste=false), and it should not pull from clipboard
  setClipboard(Some("mno"));

  wait(~name="Set configuration to pull clipboard on paste", (state: State.t) => {
    let configuration = state.configuration;
    dispatch(
      ConfigurationSet({
        ...configuration,
        default: {
          ...configuration.default,
          vimUseSystemClipboard: {
            yank: false,
            delete: false,
            paste: false,
          },
        },
      }),
    );
    runEffects();
    true;
  });

  dispatch(KeyboardInput("y"));
  dispatch(KeyboardInput("y"));
  dispatch(KeyboardInput("P"));
  runEffects();

  wait(
    ~name=
      "paste from unnamed register pulls from clipboard when ['paste'] is set",
    (state: State.t) =>
    switch (Selectors.getActiveBuffer(state)) {
    | None => false
    | Some(buf) =>
      let line = Buffer.getLine(buf, 0);
      Log.info("Current line is: |" ++ line ++ "|");
      String.equal(line, "jkl");
    }
  );
});
