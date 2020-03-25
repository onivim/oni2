/*
 * Handler.re
 *
 * Basic input handling for Oni
 */

module Log = (val Oni_Core.Log.withNamespace("Oni2.Input.Handler"));
module Zed_utf8 = Oni_Core.ZedBundled;

module Internal = {
  let keyCodeToVimString = (keycode, keyString) => {
    let len = Zed_utf8.length(keyString);
    switch (keycode) {
    | _ when len == 1 => Some(keyString)
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
    | v when v == 1073741882 => Some("F1")
    | v when v == 1073741883 => Some("F2")
    | v when v == 1073741884 => Some("F3")
    | v when v == 1073741885 => Some("F4")
    | v when v == 1073741886 => Some("F5")
    | v when v == 1073741887 => Some("F6")
    | v when v == 1073741888 => Some("F7")
    | v when v == 1073741889 => Some("F8")
    | v when v == 1073741890 => Some("F9")
    | v when v == 1073741891 => Some("F10")
    | v when v == 1073741892 => Some("F11")
    | v when v == 1073741893 => Some("F12")
    | _ => None
    };
  };

  let keyPressToString =
      (~isTextInputActive, ~altKey, ~shiftKey, ~ctrlKey, ~superKey, keycode) => {
    let keyString = Revery.Key.Keycode.getName(keycode);
    Log.trace("keyPressToString - key name: " ++ keyString);

    let keyString =
      if (!shiftKey && String.length(keyString) == 1) {
        String.lowercase_ascii(keyString);
      } else {
        keyString;
      };

    Log.tracef(m => m("Processing keycode: %i|%s", keycode, keyString));

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

    if (isKeyAllowed) {
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

        Log.trace("Sending key: " ++ ret);
        Some(ret);
      };
    } else {
      Log.trace("Key blocked: " ++ keyString);
      None;
    };
  };
};

let keyPressToCommand =
    (~isTextInputActive, {modifiers, keycode, _}: EditorInput.KeyPress.t) => {
  let altGr = modifiers.altGr;
  let shiftKey = modifiers.shift;
  let altKey = modifiers.alt;
  let ctrlKey = modifiers.control;
  let superKey = modifiers.meta;

  if (altGr && isTextInputActive) {
    None;
        // If AltGr is pressed, and we're in text input mode, we'll assume the text input handled it
  } else {
    Internal.keyPressToString(
      ~isTextInputActive,
      ~shiftKey,
      ~altKey,
      ~ctrlKey,
      ~superKey,
      keycode,
    );
  };
};
