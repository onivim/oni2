/*
 * Input.re
 *
 * Input conversion for Neovim
 */

open Revery_Core;

type t = {
    ctrlKey: bool,
    altKey: bool,
    shiftKey: bool,
    superKey: bool
};

let create = () => {
    ctrlKey: false,
    altKey: false,
    shiftKey: false,
    superKey: false,
}

let show = (state: t) => {
   "ctrlKey: " ++ string_of_bool(state.ctrlKey); 
}

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
    
    let s = ctrlKey ? "C-" ++ s : s;
    let s = shiftKey ? "S-" ++ s : s;
    let s = altKey ? "A-" ++ s : s;
    let s = superKey ? "D-" ++ s : s;

    String.length(s) > 1 ? "<" ++ s ++ ">" : s
}

let keyDown = (_state: t, evt: Events.keyEvent) => {
   let {altKey, shiftKey, ctrlKey, superKey, key, _ }: Events.keyEvent = evt;

   let c = switch(keyToCharacter(key)) {
   | Some(k) => Some(keyPressToString(~altKey, ~shiftKey, ~ctrlKey, ~superKey, k));
   | None => None;
   };

   (c, {altKey, shiftKey, ctrlKey, superKey})
};

let keyPress = (state: t, evt: Events.keyPressEvent) => {
   let {altKey, shiftKey, ctrlKey, superKey}: t = state;


   let { character, _ }: Events.keyPressEvent = evt;
   let c = keyPressToString(~altKey, ~shiftKey, ~ctrlKey, ~superKey, character);
   print_endline ("KEYPRESS: " ++ c);

   (Some(c), state)
}

let keyUp = (_state: t, evt: Events.keyEvent) => {
   let {altKey, shiftKey, ctrlKey, superKey, /*key,*/ _ }: Events.keyEvent = evt;
   (None,{altKey, shiftKey, ctrlKey, superKey}: t)
};
