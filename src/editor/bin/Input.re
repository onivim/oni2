/*
 * Input.re
 *
 * Basic input handling for Oni
 */

open Reglfw.Glfw;
open Reglfw.Glfw.Key;

let keyPressToString = (~altKey, ~shiftKey, ~ctrlKey, ~superKey, s) => {
  let s = s == "<" ? "lt" : s;

  let s = ctrlKey ? "C-" ++ s : s;
  let s = shiftKey ? "S-" ++ s : s;
  let s = altKey ? "A-" ++ s : s;
  let s = superKey ? "D-" ++ s : s;

  String.length(s) > 1 ? "<" ++ s ++ ">" : s;
};

let charToCommand = (codepoint: int, mods: Modifier.t) => {
  let char = String.make(1, Uchar.to_char(Uchar.of_int(codepoint)));

  let altKey = Modifier.isAltPressed(mods);
  let ctrlKey = Modifier.isControlPressed(mods);
  let superKey = Modifier.isSuperPressed(mods);

  let key =
    keyPressToString(~shiftKey=false, ~altKey, ~ctrlKey, ~superKey, char);
  Some(key);
};

let keyPressToCommand =
    (key: Key.t, buttonState: ButtonState.t, mods: Modifier.t) =>
  if (buttonState == GLFW_PRESS || buttonState == GLFW_REPEAT) {
    /* If ctrl is pressed, it's a non printable character, not handled by charMods - so we handle any character */
    let key =
      if (Modifier.isControlPressed(mods)) {
        switch (key) {
        | _ => Some(Key.show(key))
        };
      } else {
        switch (key) {
        | GLFW_KEY_ESCAPE => Some("ESC")
        | GLFW_KEY_TAB => Some("TAB")
        | GLFW_KEY_ENTER => Some("CR")
        | GLFW_KEY_BACKSPACE => Some("BS")
        | GLFW_KEY_LEFT => Some("LEFT")
        | GLFW_KEY_RIGHT => Some("RIGHT")
        | GLFW_KEY_DOWN => Some("DOWN")
        | GLFW_KEY_UP => Some("UP")
        | GLFW_KEY_LEFT_SHIFT
        | GLFW_KEY_RIGHT_SHIFT => Some("SHIFT")
        | _ => None
        };
      };

    switch (key) {
    | None => None
    | Some(v) =>
      let altKey = Modifier.isAltPressed(mods);
      let ctrlKey = Modifier.isControlPressed(mods);
      let superKey = Modifier.isSuperPressed(mods);
      let shiftKey = Modifier.isShiftPressed(mods);
      let keyToSend =
        keyPressToString(~shiftKey, ~altKey, ~ctrlKey, ~superKey, v);
      Some(keyToSend);
    };
  } else {
    None;
  };
