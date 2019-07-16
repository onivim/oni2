/*
 * Input.re
 *
 * Basic input handling for Oni
 */

open Oni_Core;
open Oni_Model;
open Reglfw.Glfw;
open Revery_Core;

open CamomileBundled.Camomile;
module Zed_utf8 = Oni_Core.ZedBundled;

let keyPressToString = (~altKey, ~shiftKey, ~ctrlKey, ~superKey, s) => {
  let s = s == "<" ? "lt" : s;

  let s = ctrlKey ? "C-" ++ s : s;
  let s = shiftKey ? "S-" ++ s : s;
  let s = altKey ? "A-" ++ s : s;
  let s = superKey ? "D-" ++ s : s;

  let ret = Zed_utf8.length(s) > 1 ? "<" ++ s ++ ">" : s;
  ret;
};

let isOniModifier = (~altKey, ~ctrlKey, ~superKey, ~key) => {
  let enterPressed = "<CR>" == key;
  ctrlKey || altKey || superKey || enterPressed;
};

let charToCommand = (codepoint: int, mods: Modifier.t) => {
  let char = Zed_utf8.singleton(UChar.of_int(codepoint));

  let altKey = Modifier.isAltPressed(mods);
  let ctrlKey = Modifier.isControlPressed(mods);
  let superKey = Modifier.isSuperPressed(mods);

  let key =
    keyPressToString(~shiftKey=false, ~altKey, ~ctrlKey, ~superKey, char);
  let shouldOniListen = isOniModifier(~altKey, ~ctrlKey, ~superKey, ~key);
  Some((key, shouldOniListen));
};

let keyPressToCommand =
    (
      {shiftKey, altKey, ctrlKey, superKey, key, _}: Events.keyEvent,
      os: Revery_Core.Environment.os,
    ) => {
  let commandKeyPressed =
    switch (os) {
    | Mac => superKey || ctrlKey
    | _ => ctrlKey
    };

  let keyString =
    commandKeyPressed
      ? /**
        TODO:
        Revery's toString method returns lower case
        characters which need to be capitalized. Instead we
        should use ?derving show (we will need to format out the KEY_ prefix)
        or convert the return values to uppercase
       */
        Some(Revery.Key.toString(key) |> String.capitalize_ascii)
      : (
        switch (key) {
        | KEY_ESCAPE => Some("ESC")
        | KEY_TAB => Some("TAB")
        | KEY_ENTER => Some("CR")
        | KEY_BACKSPACE => Some("C-h")
        | KEY_LEFT => Some("LEFT")
        | KEY_RIGHT => Some("RIGHT")
        | KEY_DOWN => Some("DOWN")
        | KEY_UP => Some("UP")
        | KEY_LEFT_SHIFT
        | KEY_RIGHT_SHIFT => Some("SHIFT")
        | _ => None
        }
      );
  switch (keyString) {
  | None => None
  | Some(k) =>
    let keyPressString =
      keyPressToString(~shiftKey, ~altKey, ~ctrlKey, ~superKey, k);
    let shouldOniListen =
      isOniModifier(~altKey, ~ctrlKey, ~superKey, ~key=keyPressString);

    Some((keyPressString, shouldOniListen));
  };
};

/**
   Search if any of the matching "when" conditions in the Keybindings.json
   match the current condition in state
 */
let matchesCondition = (conditions, currentMode, input, key) =>
  List.fold_left(
    (prevMatch, condition) => prevMatch || condition == currentMode,
    false,
    conditions,
  )
  |> (&&)(input == key);

let getActionsForBinding =
    (inputKey, commands, {inputControlMode, _}: State.t) =>
  Keybindings.(
    List.fold_left(
      (defaultAction, {key, command, condition}) =>
        matchesCondition(condition, inputControlMode, inputKey, key)
          ? [Actions.Command(command)] : defaultAction,
      [],
      commands,
    )
  );

/**
  Handle Input from Oni or Neovim
 */
let handle = (~state: State.t, ~commands: Keybindings.t, inputKey) => {
  switch (state.inputControlMode) {
  | CommandLineFocus
  | EditorTextFocus =>
    switch (getActionsForBinding(inputKey, commands, state)) {
    | [] =>
      Log.info("Input::handle - sending raw input: " ++ inputKey);
      [Actions.KeyboardInput(inputKey)];
    | actions =>
      Log.info("Input::handle - sending bound actions.");
      actions;
    }
  | TextInputFocus
  | MenuFocus => getActionsForBinding(inputKey, commands, state)
  };
};
