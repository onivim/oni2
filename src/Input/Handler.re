/*
 * Handler.re
 *
 * Basic input handling for Oni
 */

open Oni_Core;
open Revery;
module Log = Oni_Core.Log;

open CamomileBundled.Camomile;
module Zed_utf8 = Oni_Core.ZedBundled;

let keyCodeToVimString = (keycode, keyString) => {
  let len = Zed_utf8.length(keyString);
  switch (keycode) {
  | v when len == 1 => Some(keyString)
  | v when v == 13 /* enter */ => Some("CR")
  | v when v == Revery.Key.Keycode.escape => Some("ESC")
  | v when v == 1073741912 /*Revery.Key.Keycode.kp_enter*/ => Some("CR")
  | v when v == 9 /*Revery.Key.Keycode.tab*/ => Some("TAB")
  | v when v == Revery.Key.Keycode.backspace => Some("BS")
  | v when v == Revery.Key.Keycode.delete => Some("DEL")
  | v when v == 1073741897 => Some("INS")
  | v when v == 1073741898 => Some("HOME")
  | v when v == 1073741899 => Some("PAGEUP")
  | v when v == 1073741901 => Some("END")
  | v when v == 1073741902 => Some("PAGEDOWN")
  | v when v == 1073741903 => Some("RIGHT")
  | v when v == 1073741904 => Some("LEFT")
  | v when v == 1073741905 => Some("DOWN")
  | v when v == 1073741906 => Some("UP")
  | _ => None
  };
};

let keyPressToString =
    (~isTextInputActive, ~altKey, ~shiftKey, ~ctrlKey, ~superKey, keycode) => {
  let keyString = Revery.Key.Keycode.getName(keycode);
  Log.info("Input - keyPressToString - key name: " ++ keyString);

  let keyString =
    if (!shiftKey && String.length(keyString) == 1) {
      String.lowercase_ascii(keyString);
    } else {
      keyString;
    };

  Log.info(
    "Input - keyPressToString - processing keycode: "
    ++ string_of_int(keycode)
    ++ "|"
    ++ keyString,
  );

  let vimString = keyCodeToVimString(keycode, keyString);
  let vimStringLength =
    switch (vimString) {
    | None => 0
    | Some(v) => Zed_utf8.length(v)
    };

  let isKeyAllowed =
    isTextInputActive
      // If text input is active, only allow keys through that have modifiers
      // like control or command
      // Always allow if controlKey or superKey, and the keyString is a single character
      ? (ctrlKey || superKey)
        && Zed_utf8.length(keyString) == 1
        || vimStringLength > 1
      : true;

  switch (isKeyAllowed) {
  | false =>
    Log.info("keyPressToString - key blocked: " ++ keyString);
    None;
  | true =>
    switch (vimString) {
    | None => None
    | Some(s) =>
      let s = s == "<" ? "lt" : s;
      let s = s == "\t" ? "TAB" : s;

      let s = ctrlKey ? "C-" ++ s : s;
      let s = shiftKey ? "S-" ++ s : s;
      let s = altKey ? "A-" ++ s : s;
      let s = superKey ? "D-" ++ s : s;

      let ret = Zed_utf8.length(s) > 1 ? "<" ++ s ++ ">" : s;
      Log.info("keyPressToString - sending key: " ++ ret);
      Some(ret);
    }
  };
};

let keyPressToCommand =
    (
      ~isTextInputActive,
      {keymod, keycode, _}: Key.KeyEvent.t,
      os: Environment.os,
    ) => {
  open Revery.Key;
  let superKey = Keymod.isGuiDown(keymod);
  let shiftKey = Keymod.isShiftDown(keymod);
  let altKey = Keymod.isAltDown(keymod);
  let ctrlKey = Keymod.isControlDown(keymod);
  let altGr = Keymod.isAltGrKeyDown(keymod);

  let (altGr, ctrlKey, altKey) =
    switch (Revery.Environment.os) {
    // On Windows, we need to do some special handling here.
    // Windows has this funky behavior where pressing AltGr registers as RAlt+LControl down - more info here:
    // https://devblogs.microsoft.com/oldnewthing/?p=40003
    | Revery.Environment.Windows =>
      let altGr =
        altGr
        || Keymod.isRightAltDown(keymod)
        && Keymod.isControlDown(keymod);
      // If altGr is active, disregard control / alt key
      let ctrlKey = altGr ? false : ctrlKey;
      let altKey = altGr ? false : altKey;
      (altGr, ctrlKey, altKey);
    | _ => (altGr, ctrlKey, altKey)
    };

  let commandKeyPressed =
    switch (os) {
    | Mac => superKey || ctrlKey
    | _ => ctrlKey
    };

  if (altGr && isTextInputActive) {
    None;
        // If AltGr is pressed, and we're in text input mode, we'll assume the text input handled it
  } else {
    let keyPressString =
      keyPressToString(
        ~isTextInputActive,
        ~shiftKey,
        ~altKey,
        ~ctrlKey,
        ~superKey,
        keycode,
      );

    switch (keyPressString) {
    | None => None
    | Some(_) as v => v
    };
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
