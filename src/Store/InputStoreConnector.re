/*
 * InputStoreConnector.re
 *
 * This module connects external user input to the store.
 */

open Oni_Core;
open Oni_Input;

module Model = Oni_Model;
module State = Model.State;
module Actions = Model.Actions;

let conditionsOfState = (state: State.t) => {
  // Not functional, but we'll use the hashtable for performance
  let ret: Handler.Conditions.t = Hashtbl.create(16);

  if (state.commandline.show) {
    Hashtbl.add(ret, CommandLineFocus, true);
  };

  if (state.menu.isOpen) {
    Hashtbl.add(ret, MenuFocus, true);
  };

  // HACK: Because we don't have AND conditions yet for input
  // (the conditions array are OR's), we are making `insertMode`
  // only true when the editor is insert mode AND we are in the
  // editor (editorTextFocus is set)
  switch (state.menu.isOpen || state.commandline.show, state.mode) {
  | (false, Vim.Types.Insert) =>
    Hashtbl.add(ret, Types.Input.InsertMode, true);
    Hashtbl.add(ret, Types.Input.EditorTextFocus, true);
  | (false, _) => Hashtbl.add(ret, Types.Input.EditorTextFocus, true)
  | _ => ()
  };

  ret;
};

let start =
    (
      getState: unit => Model.State.t,
      window: Revery.Window.t,
      runEffects,
    ) => {

  let (stream, dispatch) = Isolinear.Stream.create();
  let commands = Keybindings.get();
  
  let getActionsForBinding = (inputKey, commands, currentConditions: Handler.Conditions.t) => {
    Keybindings.(
      List.fold_left(
        (defaultAction, {key, command, condition}) =>
          Handler.matchesCondition(condition, currentConditions, inputKey, key)
            ? [Actions.Command(command)] : defaultAction,
        [],
        commands,
      )
    );
  };

  /**
    Handle Input from Oni or Vim
   */
  let handle = (~isMenuOpen, ~conditions:  Handler.Conditions.t, ~time=0.0, ~commands: Keybindings.t, inputKey) => {
    let actions =
      switch (isMenuOpen) {
      | false =>
        switch (getActionsForBinding(inputKey, commands, conditions)) {
        | [] =>
          Log.info("Input::handle - sending raw input: " ++ inputKey);
          [Actions.KeyboardInput(inputKey)];
        | actions =>
          Log.info("Input::handle - sending bound actions.");
          actions;
        }
      | true => getActionsForBinding(inputKey, commands, conditions)
      };

    [Actions.NotifyKeyPressed(time, inputKey), ...actions];
  };

  /**
     The key handlers return (keyPressedString, shouldOniListen)
     i.e. if ctrl or alt or cmd were pressed then Oni2 should listen
     /respond to commands otherwise if input is alphabetical AND
     a revery element is focused oni2 should defer to revery
   */
  let keyEventListener = key => {
    let state = getState();
    let commands = state.keyBindings;
    let conditions = conditionsOfState(state);
    let time = Revery.Time.getTime() |> Revery.Time.toSeconds;
    switch (key, Revery.UI.Focus.focused) {
    | (None, _) => ()
    | (Some((k, true)), {contents: Some(_)})
    | (Some((k, _)), {contents: None}) =>
      handle(~isMenuOpen=state.menu.isOpen,
          ~conditions, ~time, ~commands, k) |> List.iter(dispatch);
      // Run input effects _immediately_
      runEffects();
    | (Some((_, false)), {contents: Some(_)}) => ()
    };
  };

  Revery.Event.subscribe(window.onKeyDown, keyEvent =>
    Handler.keyPressToCommand(keyEvent, Revery_Core.Environment.os)
    |> keyEventListener
  )
  |> ignore;

  Reglfw.Glfw.glfwSetCharModsCallback(window.glfwWindow, (_w, codepoint, mods) =>
    Handler.charToCommand(codepoint, mods) |> keyEventListener
  );
  // Noop
  stream;
};
