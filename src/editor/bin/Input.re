/*
 * Input.re
 *
 * Basic input handling for Oni
 */

open Oni_Core;
open Oni_Model;
open Reglfw.Glfw;
open Reglfw.Glfw.Key;
open Revery_Core.Events;

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

let charToCommand = ({shiftKey, altKey, ctrlKey, superKey, scancode, _}) => {
  let char = Zed_utf8.singleton(UChar.of_int(scancode));

  let key = keyPressToString(~shiftKey, ~altKey, ~ctrlKey, ~superKey, char);
  Some(key);
};

let keyPressToCommand = (event: Revery_Core.Events.keyPressEvent) =>
  /* If ctrl is pressed, it's a non printable character, not handled by charMods - so we handle any character */
  Some(event.character);
/* let key = */
/*   if (Modifier.isControlPressed(mods)) { */
/*     switch (key) { */
/*     | _ => Some(Key.show(key)) */
/*     }; */
/*   } else { */
/*     switch (key) { */
/*     | GLFW_KEY_ESCAPE => Some("ESC") */
/*     | GLFW_KEY_TAB => Some("TAB") */
/*     | GLFW_KEY_ENTER => Some("CR") */
/*     | GLFW_KEY_BACKSPACE => Some("BS") */
/*     | GLFW_KEY_LEFT => Some("LEFT") */
/*     | GLFW_KEY_RIGHT => Some("RIGHT") */
/*     | GLFW_KEY_DOWN => Some("DOWN") */
/*     | GLFW_KEY_UP => Some("UP") */
/*     | GLFW_KEY_LEFT_SHIFT */
/*     | GLFW_KEY_RIGHT_SHIFT => Some("SHIFT") */
/*     | _ => None */
/*     }; */
/*   }; */
/* switch (key) { */
/* | None => None */
/* | Some(v) => */
/*   let altKey = Modifier.isAltPressed(mods); */
/*   let ctrlKey = Modifier.isControlPressed(mods); */
/*   let superKey = Modifier.isSuperPressed(mods); */
/*   let shiftKey = Modifier.isShiftPressed(mods); */
/*   let keyToSend = */
/*     keyPressToString(~shiftKey, ~altKey, ~ctrlKey, ~superKey, v); */
/*   Some(keyToSend); */
/* }; */

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
