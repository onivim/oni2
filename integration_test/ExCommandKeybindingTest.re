open Oni_Model;
open Oni_IntegrationTestLib;

let keybindings =
  Some(
    {|
[
  {"key": "kk", "command": ":split", "when": "editorTextFocus"}
]
|},
  );

runTest(
  ~keybindings,
  ~name="ExCommandKeybindingTest",
  ({wait, input, _}) => {
    wait(~name="Initial sanity check", (state: State.t) => {
      let splitCount =
        state.layout |> Feature_Layout.visibleEditors |> List.length;

      splitCount == 1;
    });

    input("k");
    input("k");

    wait(~name="Wait for split to be created", (state: State.t) => {
      let splitCount =
        state.layout |> Feature_Layout.visibleEditors |> List.length;

      splitCount == 2;
    });
  },
);
