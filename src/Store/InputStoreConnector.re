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
      window: option(Revery.Window.t),
      runEffects,
    ) => {
  let (stream, dispatch) = Isolinear.Stream.create();

  // We also use 'text input' mode for SDL2.
  // This enables us to get resolved keyboard events, and IME.

  // For IME: Is this sufficient? Or will we need a way to turn off / toggle IME when switching modes?
  Sdl2.TextInput.start();

  let getActionsForBinding =
      (inputKey, commands, currentConditions: Handler.Conditions.t) => {
    let inputKey = String.uppercase_ascii(inputKey);
    Keybindings.(
      List.fold_left(
        (defaultAction, {key, command, condition}) =>
          Handler.matchesCondition(
            condition,
            currentConditions,
            inputKey,
            key,
          )
            ? [Actions.Command(command)] : defaultAction,
        [],
        commands,
      )
    );
  };

  /**
    Handle Input from Oni or Vim
   */
  let handle =
      (
        ~isMenuOpen,
        ~conditions: Handler.Conditions.t,
        ~time=0.0,
        ~commands: Keybindings.t,
        inputKey,
      ) => {
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
    // No key, nothing focused - no-op
    | (None, _) => ()

    // We have a key, but Revery has an element focused
    | (Some(k), {contents: Some(_)})
    | (Some(k), {contents: None}) =>
      handle(~isMenuOpen=state.menu.isOpen, ~conditions, ~time, ~commands, k)
      |> List.iter(dispatch);

      // Run input effects _immediately_
      runEffects();
    };
  };

  let isTextInputActive = () => {
    switch (window) {
    | None => false
    | Some(v) => Revery.Window.isTextInputActive(v)
    };
  };

  switch (window) {
  | None => Log.info("Input - no window to subscribe to events")
  | Some(window) =>
    let _ignore =
      Revery.Event.subscribe(
        window.onKeyDown,
        keyEvent => {
          let isTextInputActive = isTextInputActive();
          Log.info(
            "Input - got keydown - text input:"
            ++ string_of_bool(isTextInputActive),
          );
          Handler.keyPressToCommand(
            ~isTextInputActive,
            keyEvent
          )
          |> keyEventListener;
        },
      );

    let _ignore =
      Revery.Event.subscribe(
        window.onTextInputCommit,
        textEvent => {
          Log.info("Input - onTextInputCommit: " ++ textEvent.text);
          keyEventListener(Some(textEvent.text));
        },
      );
    ();
  };

  stream;
};
