/*
 * Input.re
 *
 * Basic input handling for Oni
 */

open Oni_Core;
open Oni_Model;
open Reglfw.Glfw;
open Revery_Core;

open CamomileLibraryDefault.Camomile;

let keyPressToString = (~altKey, ~shiftKey, ~ctrlKey, ~superKey, s) => {
  let s = s == "<" ? "lt" : s;

  let s = ctrlKey ? "C-" ++ s : s;
  let s = shiftKey ? "S-" ++ s : s;
  let s = altKey ? "A-" ++ s : s;
  let s = superKey ? "D-" ++ s : s;

  let ret = Zed_utf8.length(s) > 1 ? "<" ++ s ++ ">" : s;
  ret;
};

let charToCommand = (codepoint: int, mods: Modifier.t) => {
  let char = Zed_utf8.singleton(UChar.of_int(codepoint));

  let altKey = Modifier.isAltPressed(mods);
  let ctrlKey = Modifier.isControlPressed(mods);
  let superKey = Modifier.isSuperPressed(mods);

  let key =
    keyPressToString(~shiftKey=false, ~altKey, ~ctrlKey, ~superKey, char);
  let shortcutModPressed = ctrlKey || altKey || superKey;
  Some((key, shortcutModPressed));
};

let keyPressToCommand =
    ({shiftKey, altKey, ctrlKey, superKey, key, _}: Events.keyEvent) => {
  let keyString =
    ctrlKey ?
      /**
        TODO: currently Revery's toString method returns lower case
        characters which need to be capitalized. Instead we
        should use ?derving show (we will need to format out the KEY_ prefix)
        or convert the return values to uppercase
       */
      Some(Revery.Key.toString(key) |> String.capitalize_ascii) :
      (
        switch (key) {
        | KEY_ESCAPE => Some("ESC")
        | KEY_TAB => Some("TAB")
        | KEY_ENTER => Some("CR")
        | KEY_BACKSPACE => Some("BS")
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
    let res = keyPressToString(~shiftKey, ~altKey, ~ctrlKey, ~superKey, k);
    let shortcutModPressed = ctrlKey || altKey || superKey;
    Some((res, shortcutModPressed));
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
        matchesCondition(condition, inputControlMode, inputKey, key) ?
          Commands.handleCommand(command) : defaultAction,
      [],
      commands,
    )
  );

/**
  Handle Input from Oni or Neovim

 */
let handle =
    (
      ~api: Oni_Neovim.NeovimProtocol.t,
      ~state: State.t,
      ~commands: Keybindings.t,
      inputKey,
    ) =>
  switch (state.inputControlMode) {
  | EditorTextFocus =>
    switch (getActionsForBinding(inputKey, commands, state)) {
    | [] as default =>
      api.input(inputKey) |> ignore;
      default;
    | actions => actions
    }
  | TextInputFocus
  | MenuFocus => getActionsForBinding(inputKey, commands, state)
  };
