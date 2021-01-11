open Oni_Model;
open Oni_IntegrationTestLib;

runTest(~name="Viml Remap ; -> :", (dispatch, wait, runEffects) => {
  wait(~name="Initial mode is normal", (state: State.t) =>
    Selectors.mode(state) |> Vim.Mode.isNormal
  );

  dispatch(
    VimExecuteCommand({allowAnimation: true, command: "nnoremap ; :"}),
  );
  runEffects();

  let input = key => {
    let modifiers = EditorInput.Modifiers.none;

    let keyPress =
      EditorInput.KeyPress.physicalKey(
        ~key=EditorInput.Key.Character(key),
        ~modifiers,
      )
      |> EditorInput.KeyCandidate.ofKeyPress;
    let time = Revery.Time.now();

    dispatch(Model.Actions.KeyDown({key: keyPress, scancode: 1, time}));
    dispatch(Model.Actions.KeyUp({scancode: 1, time}));
    runEffects();
  };

  // Because of our remap, the ';' semicolon
  // is mapped to ':' - so sending it should open the command line.
  input(';');

  wait(~name="Mode switches to command line", (state: State.t) => {
    Selectors.mode(state) == Vim.Mode.CommandLine
  });

  input('e');

  wait(~name="'e' key is entered", (state: State.t) =>
    switch (state.quickmenu) {
    | Some(quickmenu) =>
      quickmenu.inputText |> Component_InputText.value == "e"
    | None => false
    }
  );

  input('h');

  wait(~name="'h' key is entered", (state: State.t) =>
    switch (state.quickmenu) {
    | Some(quickmenu) =>
      quickmenu.inputText |> Component_InputText.value == "eh"
    | None => false
    }
  );
});
