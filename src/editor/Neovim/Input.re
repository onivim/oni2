/*
 * Input.re
 *
 * Input conversion for Neovim
 */

open Revery_Core;

type t = string;

let keyToCharacter = (key: Key.t) => switch(key) {
      | Key.KEY_TAB => Some("TAB")
      | Key.KEY_BACKSPACE => Some("BS")
      | Key.KEY_ENTER => Some("CR")
      | Key.KEY_ESCAPE => Some("ESC")
      | Key.KEY_RIGHT_SHIFT
      | Key.KEY_LEFT_SHIFT => Some("SHIFT")
      | Key.KEY_UP => Some("UP")
      | Key.KEY_LEFT => Some("LEFT")
      | Key.KEY_RIGHT => Some("RIGHT")
      | Key.KEY_DOWN => Some("DOWN")
      | _ => None
};

let keyPressToString = (~altKey, ~shiftKey, ~ctrlKey, ~superKey, s) => {
    let s = s == "<" ? "lt" : s;
    
    let s = ctrlKey ? "c-" ++ s : s;
    let s = shiftKey ? "s-" ++ s : s;
    let s = altKey ? "a-" ++ s : s;
    let s = superKey ? "m-" ++ s : s;

    String.length(s) > 1 ? "<" ++ s ++ ">" : s
}

let ofKeyEvent = (evt: Events.keyEvent) => {
   let {altKey, shiftKey, ctrlKey, superKey, key, _ }: Events.keyEvent = evt;
   switch(keyToCharacter(key)) {
   | Some(k) => Some(keyPressToString(~altKey, ~shiftKey, ~ctrlKey, ~superKey, k))
   | None => None
   }
};

let ofKeyPressEvent = (evt: Events.keyPressEvent) => {
   /* TODO: Track this */
   let (altKey, shiftKey, ctrlKey, superKey) = (false, false, false, false);

   let { character, _ }: Events.keyPressEvent = evt;
   keyPressToString(~altKey, ~shiftKey, ~ctrlKey, ~superKey, character);
};
