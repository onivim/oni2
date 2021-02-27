open Oni_Model;
open Oni_IntegrationTestLib;

// We'll turn on 'single-file' zen mode, which means when the editor
// is opened with only one file, we'll enter zen mode automatically
let configuration = {|
{ "editor.zenMode.singleFile": true }
|};

runTest(
  ~cli=Oni_CLI.{...default, filesToOpen: ["some-random-file.txt"]},
  ~configuration=Some(configuration),
  ~name="ZenMode: Single-file mode works as expected",
  ({wait, _}) => {
    wait(~name="Wait for split to be created 1", (state: State.t) => {
      let splitCount =
        state.layout |> Feature_Layout.visibleEditors |> List.length;
      splitCount == 1;
    });

    // Verify we've entered zen-mode
    wait(~name="We should be in zen-mode", (state: State.t) =>
      Feature_Zen.isZen(state.zen)
    );
  },
);
