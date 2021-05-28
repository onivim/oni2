open Oni_Core;
open Oni_Model;
open Oni_IntegrationTestLib;

let keybindings =
  Some(
    {|
[
  {"key": "kk", "command": ":d 2", "when": "editorTextFocus"}
]
|},
  );

runTest(
  ~keybindings,
  ~name="ExCommandKeybindingTest",
  ({dispatch, wait, input, _}) => {
    let testFile = getAssetPath("some-test-file.txt");
    dispatch(Actions.OpenFileByPath(testFile, SplitDirection.Current, None));

    wait(~name="Verify buffer is loaded", (state: State.t) =>
      switch (Selectors.getActiveBuffer(state)) {
      | Some(buffer) =>
        Buffer.getShortFriendlyName(buffer) == Some("some-test-file.txt")
      | None => false
      }
    );

    wait(~name="Verify initial line count", (state: State.t) =>
      switch (Selectors.getActiveBuffer(state)) {
      | Some(buffer) => Buffer.getNumberOfLines(buffer) == 3
      | None => false
      }
    );

    input("k");
    input("k");

    wait(~name="Wait for split to be created", (state: State.t) =>
      switch (Selectors.getActiveBuffer(state)) {
      | Some(buffer) => Buffer.getNumberOfLines(buffer) == 1
      | None => false
      }
    );
  },
);
