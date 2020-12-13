open Oni_Model;
open Oni_IntegrationTestLib;
open Actions;

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
  (dispatch, wait, _) => {
    let input = key => {
      let keyPress =
        EditorInput.KeyPress.physicalKey(
          ~scancode=Sdl2.Scancode.ofName(key),
          ~keycode=Sdl2.Keycode.ofName(key),
          ~modifiers=EditorInput.Modifiers.none,
        );
      let time = Revery.Time.now();

      dispatch(KeyDown(keyPress, time));
      //dispatch(TextInput(key));
      dispatch(KeyUp(keyPress, time));
    };

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
