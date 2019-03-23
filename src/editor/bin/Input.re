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
  Some(key);
};

let keyPressToCommand =
    ({shiftKey, altKey, ctrlKey, superKey, key, _}: Events.keyEvent) => {
  let keyString =
    switch (key) {
    | KEY_SPACE => Some("SPACE")
    | KEY_APOSTROPHE => Some("'")
    | KEY_COMMA => Some(",")
    | KEY_MINUS => Some("-")
    | KEY_PERIOD => Some(".")
    | KEY_SLASH => Some("/")
    | KEY_0 => Some("0")
    | KEY_1 => Some("1")
    | KEY_2 => Some("2")
    | KEY_3 => Some("3")
    | KEY_4 => Some("4")
    | KEY_5 => Some("5")
    | KEY_6 => Some("6")
    | KEY_7 => Some("7")
    | KEY_8 => Some("8")
    | KEY_9 => Some("9")
    | KEY_SEMICOLON => None
    | KEY_EQUAL => None
    | KEY_A => None
    | KEY_B => None
    | KEY_C => None
    | KEY_D => None
    | KEY_E => None
    | KEY_F => None
    | KEY_G => None
    | KEY_H => None
    | KEY_I => None
    | KEY_J => None
    | KEY_K => None
    | KEY_L => None
    | KEY_M => None
    | KEY_N => None
    | KEY_O => None
    | KEY_P => None
    | KEY_Q => None
    | KEY_R => None
    | KEY_S => None
    | KEY_T => None
    | KEY_U => None
    | KEY_V => None
    | KEY_W => None
    | KEY_X => None
    | KEY_Y => None
    | KEY_Z => None
    | KEY_LEFT_BRACKET => Some("[")
    | KEY_BACKSLASH => Some("\\")
    | KEY_RIGHT_BRACKET => Some("]")
    | KEY_ESCAPE => Some("ESC")
    | KEY_ENTER => Some("CR")
    | KEY_TAB => Some("TAB")
    | KEY_BACKSPACE => Some("BS")
    | KEY_DELETE => Some("DEL")
    | KEY_RIGHT => Some("RIGHT")
    | KEY_LEFT => Some("LEFT")
    | KEY_DOWN => Some("RIGHT")
    | KEY_UP => Some("UP")
    | KEY_CAPS_LOCK => Some("CAPSLOCK")
    | KEY_UNKNOWN => None
    | KEY_GRAVE_ACCENT
    | KEY_WORLD_1
    | KEY_WORLD_2
    | KEY_SCROLL_LOCK
    | KEY_NUM_LOCK
    | KEY_PRINT_SCREEN
    | KEY_PAUSE
    | KEY_INSERT
    | KEY_PAGE_UP
    | KEY_PAGE_DOWN
    | KEY_HOME
    | KEY_END
    | KEY_F1
    | KEY_F2
    | KEY_F3
    | KEY_F4
    | KEY_F5
    | KEY_F6
    | KEY_F7
    | KEY_F8
    | KEY_F9
    | KEY_F10
    | KEY_F11
    | KEY_F12
    | KEY_F13
    | KEY_F14
    | KEY_F15
    | KEY_F16
    | KEY_F17
    | KEY_F18
    | KEY_F19
    | KEY_F20
    | KEY_F21
    | KEY_F22
    | KEY_F23
    | KEY_F24
    | KEY_F25
    | KEY_KP_0
    | KEY_KP_1
    | KEY_KP_2
    | KEY_KP_3
    | KEY_KP_4
    | KEY_KP_5
    | KEY_KP_6
    | KEY_KP_7
    | KEY_KP_8
    | KEY_KP_9
    | KEY_KP_DECIMAL
    | KEY_KP_DIVIDE
    | KEY_KP_MULTIPLY
    | KEY_KP_SUBTRACT
    | KEY_KP_ADD
    | KEY_KP_ENTER
    | KEY_KP_EQUAL
    | KEY_LEFT_SHIFT
    | KEY_LEFT_CONTROL
    | KEY_LEFT_ALT
    | KEY_LEFT_SUPER
    | KEY_RIGHT_SHIFT
    | KEY_RIGHT_CONTROL
    | KEY_RIGHT_ALT
    | KEY_RIGHT_SUPER
    | KEY_MENU => None
    };
  switch (keyString) {
  | None => None
  | Some(k) =>
    let res = keyPressToString(~shiftKey, ~altKey, ~ctrlKey, ~superKey, k);
    Some(res);
  };
};

let getActionsForBinding = (inputKey, commands, state: State.t) =>
  Keybindings.(
    List.fold_left(
      (defaultAction, {key, command, condition}) =>
        if (inputKey == key && condition == state.inputControlMode) {
          Commands.handleCommand(command);
        } else {
          defaultAction;
        },
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
  | MenuFocus => getActionsForBinding(inputKey, commands, state)
  };
