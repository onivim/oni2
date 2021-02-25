/*
 ClipboardNormalModePasteTest

 Test cases for pasting in normal mode:
 - '*' and '+' registers should always paste from clipboard, regardless of configuration
 - named registers (ie, 'a') should _never_ paste from clipboard, regardless of configuration
 - unnamed register should only paste from clipboard if `["paste"]` is set in configuration
 */

open Oni_Core;
open Oni_Model;
open Oni_IntegrationTestLib;

module Log = (
  val Log.withNamespace("IntegrationTest.ClipboardNormalModePaste")
);

runTest(
  ~name="InsertMode test - effects batched to runEffects",
  ({dispatch, wait, runEffects, input, _}) => {
  wait(~name="Initial mode is normal", (state: State.t) =>
    Selectors.mode(state) |> Vim.Mode.isNormal
  );

  // '*' test case
  setClipboard(Some("abc\n"));

  input("\"");
  input("*");
  input("P");

  runEffects();

  wait(~name="Mode switches to insert", (state: State.t) =>
    switch (Selectors.getActiveBuffer(state)) {
    | None => false
    | Some(buf) =>
      let line = Buffer.getLine(0, buf) |> BufferLine.raw;
      Log.info("Current line is: |" ++ line ++ "|");
      String.equal(line, "abc");
    }
  );

  // '*' multi-line test case
  setClipboard(Some("1\n2\n3\n"));

  input("\"");
  input("*");
  input("P");

  runEffects();

  wait(~name="Multi-line paste works correctly", (state: State.t) =>
    switch (Selectors.getActiveBuffer(state)) {
    | None => false
    | Some(buf) =>
      let line = Buffer.getLine(0, buf) |> BufferLine.raw;
      Log.info("Current line is: |" ++ line ++ "|");
      let a = String.equal(line, "1");

      let line = Buffer.getLine(1, buf) |> BufferLine.raw;
      Log.info("Current line is: |" ++ line ++ "|");
      let b = String.equal(line, "2");

      let line = Buffer.getLine(2, buf) |> BufferLine.raw;
      Log.info("Current line is: |" ++ line ++ "|");
      let c = String.equal(line, "3");
      a && b && c;
    }
  );

  // '*' multi-line test case, windows style
  setClipboard(Some("4\r\n5\r\n6\r\n"));

  input("\"");
  input("*");
  input("P");
  runEffects();

  wait(~name="Multi-line paste works correctly", (state: State.t) =>
    switch (Selectors.getActiveBuffer(state)) {
    | None => false
    | Some(buf) =>
      let line = Buffer.getLine(0, buf) |> BufferLine.raw;
      Log.info("Current line is: |" ++ line ++ "|");
      let a = String.equal(line, "4");

      let line = Buffer.getLine(1, buf) |> BufferLine.raw;
      Log.info("Current line is: |" ++ line ++ "|");
      let b = String.equal(line, "5");

      let line = Buffer.getLine(2, buf) |> BufferLine.raw;
      Log.info("Current line is: |" ++ line ++ "|");
      let c = String.equal(line, "6");
      a && b && c;
    }
  );
  // '+' test case
  setClipboard(Some("def\n"));

  input("\"");
  input("*");
  input("P");
  runEffects();

  wait(~name="Mode switches to insert", (state: State.t) =>
    switch (Selectors.getActiveBuffer(state)) {
    | None => false
    | Some(buf) =>
      let line = Buffer.getLine(0, buf) |> BufferLine.raw;
      Log.info("Current line is: |" ++ line ++ "|");
      String.equal(line, "def");
    }
  );

  // 'a' test case
  // yank current line - def - to 'a' register
  input("\"");
  input("a");
  input("y");
  input("y");
  runEffects();

  setClipboard(Some("ghi\n"));

  input("\"");
  input("a");
  input("P");
  runEffects();

  wait(~name="Mode switches to insert", (state: State.t) =>
    switch (Selectors.getActiveBuffer(state)) {
    | None => false
    | Some(buf) =>
      let line = Buffer.getLine(0, buf) |> BufferLine.raw;
      Log.info("Current line is: |" ++ line ++ "|");
      String.equal(line, "def");
    }
  );

  // Test if the configuration is set - paste from unnamed register will pull from the keyboard
  setClipboard(Some("jkl\n"));
  wait(~name="Set clipboard to pull clipboard on paste", _ => {
    let transformer =
      ConfigurationTransformer.setField(
        "vim.useSystemClipboard",
        `List([`String("paste")]),
      );
    dispatch(
      Configuration(Feature_Configuration.Testing.transform(transformer)),
    );
    runEffects();
    true;
  });

  wait(~name="Wait for configuration to update (1)", (state: State.t) => {
    Feature_Configuration.Legacy.getValue(
      c => c.vimUseSystemClipboard,
      state.config,
    )
    == Feature_Configuration.LegacyConfigurationValues.{
         yank: false,
         delete: false,
         paste: true,
       }
  });

  input("y");
  input("y");
  input("P");
  runEffects();

  wait(
    ~name=
      "paste from unnamed register pulls from clipboard when ['paste'] is set",
    (state: State.t) =>
    switch (Selectors.getActiveBuffer(state)) {
    | None => false
    | Some(buf) =>
      let line = Buffer.getLine(0, buf) |> BufferLine.raw;
      Log.info("Current line is: |" ++ line ++ "|");
      String.equal(line, "jkl");
    }
  );

  // Set configuration back (paste=false), and it should not pull from clipboard
  setClipboard(Some("mno\n"));

  wait(~name="Set clipboard to not pull clipboard on paste", _ => {
    let transformer =
      ConfigurationTransformer.setField(
        "vim.useSystemClipboard",
        `Assoc([
          ("yank", `Bool(false)),
          ("delete", `Bool(false)),
          ("paste", `Bool(false)),
        ]),
      );
    dispatch(
      Configuration(Feature_Configuration.Testing.transform(transformer)),
    );
    runEffects();
    true;
  });

  input("y");
  input("y");
  input("P");
  runEffects();

  wait(
    ~name=
      "paste from unnamed register pulls from clipboard when ['paste'] is set",
    (state: State.t) =>
    switch (Selectors.getActiveBuffer(state)) {
    | None => false
    | Some(buf) =>
      let line = Buffer.getLine(0, buf) |> BufferLine.raw;
      Log.info("Current line is: |" ++ line ++ "|");
      String.equal(line, "jkl");
    }
  );

  // Single line case - should paste in front of previous text
  setClipboard(Some("mno"));

  input("\"");
  input("*");
  input("P");
  runEffects();

  wait(~name="paste with single line, from clipboard", (state: State.t) =>
    switch (Selectors.getActiveBuffer(state)) {
    | None => false
    | Some(buf) =>
      let line = Buffer.getLine(0, buf) |> BufferLine.raw;
      Log.info("Current line is: |" ++ line ++ "|");
      String.equal(line, "mnojkl");
    }
  );
});
