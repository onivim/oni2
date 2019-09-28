/*
 * Handler.re
 *
 * Basic input handling for Oni
 */

open Oni_Core;
open Reglfw.Glfw;
open Revery;
module Log = Oni_Core.Log;

open CamomileBundled.Camomile;
module Zed_utf8 = Oni_Core.ZedBundled;

let keyPressToString = (~altKey, ~shiftKey, ~ctrlKey, ~superKey, s) => {
  let s = s == "<" ? "lt" : s;
  let s = s == "\t" ? "TAB" : s;

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
      {keymod, keycode, _}: Key.KeyEvent.t,
      os: Environment.os,
    ) => {
  open Revery.Key;
  let superKey = Keymod.isGuiDown(keymod);
  let shiftKey = Keymod.isShiftDown(keymod);
  let altKey = Keymod.isAltDown(keymod);
  let ctrlKey = Keymod.isControlDown(keymod);

  let commandKeyPressed =
    switch (os) {
    | Mac => superKey || ctrlKey
    | _ => ctrlKey
    };

  let keyString = Some(Revery.Key.Keycode.getName(keycode) |> String.capitalize_ascii);
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

module Conditions = {
  type t = Hashtbl.t(Types.Input.controlMode, bool);

  let getBooleanCondition = (v: t, condition: Types.Input.controlMode) => {
    switch (Hashtbl.find_opt(v, condition)) {
    | Some(v) => v
    | None => false
    };
  };
  /*let ofState = (state: State.t) => {
      // Not functional, but we'll use the hashtable for performance
      let ret: t = Hashtbl.create(16);

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
    };*/
};

/**
   Search if any of the matching "when" conditions in the Keybindings.json
   match the current condition in state
 */
let matchesCondition = (commandConditions, currentConditions, input, key) =>
  if (input != key) {
    false;
  } else {
    List.fold_left(
      (prevMatch, condition) =>
        prevMatch
        || Conditions.getBooleanCondition(currentConditions, condition),
      false,
      commandConditions,
    );
  };
