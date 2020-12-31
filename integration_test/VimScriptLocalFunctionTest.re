open Oni_Model;
open Oni_IntegrationTestLib;

runTest(~name="VimScriptLocalFunctionTest", (dispatch, wait, runEffects) => {
  wait(~name="Initial mode is normal", (state: State.t) =>
    Selectors.mode(state) |> Vim.Mode.isNormal
  );

  let plugScript = getAssetPath("PlugScriptLocal.vim");

  dispatch(
    VimExecuteCommand({
      allowAnimation: true,
      command: "source " ++ plugScript,
    }),
  );
  runEffects();

  // set up a binding to the <Plug>Hello1 command provided by the script
  dispatch(
    VimExecuteCommand({
      allowAnimation: true,
      command: "nnoremap j <Plug>Hello1",
    }),
  );
  runEffects();

  let input = key => {
    let modifiers = EditorInput.Modifiers.none;

    let keyPress: EditorInput.KeyPress.t =
      EditorInput.KeyPress.physicalKey(
        ~key=EditorInput.Key.Character(key),
        ~modifiers,
      );
    let time = Revery.Time.now();

    dispatch(Model.Actions.KeyDown({key: keyPress, scancode: 1, time}));
    dispatch(Model.Actions.KeyUp({key: keyPress, scancode: 1, time}));
    runEffects();
  };

  input('j');

  wait(~name="plugin notification shows up", (state: State.t) => {
    let notifications = Feature_Notification.all(state.notifications);
    if (notifications != []) {
      let notification = List.hd(notifications);
      String.equal(notification.message, "hello1");
    } else {
      false;
    };
  });
});
