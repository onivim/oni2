/*
 * Handler.re
 *
 * Basic input handling for Oni
 */

module Log = (val Oni_Core.Log.withNamespace("Oni2.Input.Handler"));
module Zed_utf8 = Oni_Core.ZedBundled;

open Oni_Core.Utility;

module Internal = {
  let keyToVimString = key => {
    EditorInput.Key.(
      {
        switch (key) {
        | Character(char) => Some(String.make(1, char))
        | Return => Some("CR")
        | Escape => Some("ESC")
        | Tab => Some("TAB")
        | Backspace => Some("BS")
        | Delete => Some("DEL")
        | Insert => Some("INS")
        | Home => Some("HOME")
        | PageUp => Some("PAGEUP")
        | End => Some("END")
        | PageDown => Some("PAGEDOWN")
        | Right => Some("RIGHT")
        | Left => Some("LEFT")
        | Down => Some("DOWN")
        | Up => Some("UP")
        | Function(num) => Some("F" ++ string_of_int(num))
        | Space => Some(" ")
        | Pause => Some("PAUSE")
        | _ => None
        };
      }
    );
  };

  let keyPressToString =
      (
        ~force,
        ~isTextInputActive,
        ~altKey,
        ~shiftKey,
        ~ctrlKey,
        ~superKey,
        key,
      ) => {
    let keyString = EditorInput.Key.toString(key);
    Log.trace("keyPressToString - key name: " ++ keyString);

    let keyString =
      if (!shiftKey && String.length(keyString) == 1) {
        String.lowercase_ascii(keyString);
      } else {
        keyString;
      };

    Log.tracef(m => m("Processing key: %s", keyString));

    let vimString = keyToVimString(key);
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
          || force
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

let keyPressToCommand = (~force, ~isTextInputActive, key) => {
  let maybeKey = EditorInput.KeyPress.toPhysicalKey(key);
  maybeKey
  |> OptionEx.flatMap(({modifiers, key}: EditorInput.PhysicalKey.t) => {
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
           ~force,
           ~isTextInputActive,
           ~shiftKey,
           ~altKey,
           ~ctrlKey,
           ~superKey,
           key,
         );
       };
     });
};
