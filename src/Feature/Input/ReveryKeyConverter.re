open Oni_Core;
open Utility;

module Log = (val Log.withNamespace("Feature.Input.ReveryKeyConverter"));

module Internal = {
  let tryToGetSpecialKey = keycode => {
    EditorInput.(
      switch (keycode) {
      | v when v == 13 /* enter */ => Some(Key.Return)
      | v when v == Revery.Key.Keycode.escape => Some(Key.Escape)
      | v when v == 1073741912 /*Revery.Key.Keycode.kp_enter*/ =>
        Some(Key.Return)
      | v when v == 9 /*Revery.Key.Keycode.tab*/ => Some(Key.Tab)
      | v when v == Revery.Key.Keycode.backspace => Some(Key.Backspace)
      | v when v == Revery.Key.Keycode.delete => Some(Key.Delete)
      | v when v == Revery.Key.Keycode.space => Some(Key.Space)
      | v when v == 1073741881 => Some(Key.CapsLock)
      | v when v == 1073741897 => Some(Key.Insert)
      | v when v == 1073741898 => Some(Key.Home)
      | v when v == 1073741899 => Some(Key.PageUp)
      | v when v == 1073741901 => Some(Key.End)
      | v when v == 1073741902 => Some(Key.PageDown)
      | v when v == 1073741903 => Some(Key.Right)
      | v when v == 1073741904 => Some(Key.Left)
      | v when v == 1073741905 => Some(Key.Down)
      | v when v == 1073741906 => Some(Key.Up)
      | v when v == 1073741882 => Some(Key.Function(1))
      | v when v == 1073741883 => Some(Key.Function(2))
      | v when v == 1073741884 => Some(Key.Function(3))
      | v when v == 1073741885 => Some(Key.Function(4))
      | v when v == 1073741886 => Some(Key.Function(5))
      | v when v == 1073741887 => Some(Key.Function(6))
      | v when v == 1073741888 => Some(Key.Function(7))
      | v when v == 1073741889 => Some(Key.Function(8))
      | v when v == 1073741890 => Some(Key.Function(9))
      | v when v == 1073741891 => Some(Key.Function(10))
      | v when v == 1073741892 => Some(Key.Function(11))
      | v when v == 1073741893 => Some(Key.Function(12))
      | v when v == 1073742048 => Some(Key.LeftControl)
      | v when v == 1073742052 => Some(Key.RightControl)
      | _ => None
      }
    );
  };

  let tryToGetCharacterFromKey =
      (~shift, ~control, ~alt, ~super, ~altGr, ~scancode) => {
    let keyboard = Oni2_KeyboardLayout.Keymap.getCurrent();
    let maybeKeymap =
      Oni2_KeyboardLayout.Keymap.entryOfScancode(keyboard, scancode);

    switch (maybeKeymap) {
    | Some(keymap) =>
      Log.infof(m =>
        m("Key info: %s\n", Oni2_KeyboardLayout.Keymap.entryToString(keymap))
      )
    | None => Log.info("No keymap for key.")
    };

    let stringToKey = (maybeString: string) => {
      let maybeChar =
        try(Some(ZedBundled.get(maybeString, 0))) {
        | _exn => None
        };
      maybeChar |> Option.map(char => EditorInput.Key.Character(char));
    };

    maybeKeymap
    |> OptionEx.tap(keymap => {
         Log.infof(m =>
           m(
             "Key info: %s\n",
             Oni2_KeyboardLayout.Keymap.entryToString(keymap),
           )
         )
       })
    |> OptionEx.tapNone(() => Log.info("No keymap for key"))
    |> OptionEx.flatMap((keymap: Oni2_KeyboardLayout.Keymap.entry) => {
         open EditorInput;
         open KeyPress;
         open Modifiers;

         let modifiers = {shift, control, alt, super, altGr};
         let defaultCandidate =
           keymap.unmodified
           |> OptionEx.flatMap(stringToKey)
           |> Option.map(key => physicalKey(~key, ~modifiers));

         let maybeShiftAltGrCandidate =
           if (shift && altGr) {
             keymap.withAltGraphShift
             |> OptionEx.flatMap(stringToKey)
             |> Option.map(key =>
                  physicalKey(
                    ~key,
                    ~modifiers={...modifiers, shift: false, altGr: false},
                  )
                );
           } else {
             None;
           };

         let maybeShiftCandidate =
           if (shift) {
             keymap.withShift
             |> OptionEx.flatMap(stringToKey)
             |> Option.map(key =>
                  physicalKey(~key, ~modifiers={...modifiers, shift: false})
                );
           } else {
             None;
           };

         let maybeAltGrCandidate =
           if (altGr) {
             keymap.withAltGraph
             |> OptionEx.flatMap(stringToKey)
             |> Option.map(key =>
                  physicalKey(~key, ~modifiers={...modifiers, altGr: false})
                );
           } else {
             None;
           };

         let candidates =
           [
             defaultCandidate,
             maybeShiftAltGrCandidate,
             maybeShiftCandidate,
             maybeAltGrCandidate,
           ]
           |> List.filter_map(Fun.id);

         if (candidates == []) {
           None;
         } else {
           Some(candidates);
         };
       });
  };
};
let reveryKeyToKeyPress =
    ({scancode, keycode, keymod, repeat, _}: Revery.Key.KeyEvent.t) => {
  // TODO: Should we filter out repeat keys from key binding processing?
  ignore(repeat);
  let name = Sdl2.Scancode.ofInt(scancode) |> Sdl2.Scancode.getName;

  if (name == "Left Shift" || name == "Right Shift") {
    None;
  } else {
    let shift = Revery.Key.Keymod.isShiftDown(keymod);
    let control = Revery.Key.Keymod.isControlDown(keymod);
    let alt = Revery.Key.Keymod.isAltDown(keymod);
    let super = Revery.Key.Keymod.isGuiDown(keymod);
    let altGr = Revery.Key.Keymod.isAltGrKeyDown(keymod);

    let (altGr, control, alt) =
      switch (Revery.Environment.os) {
      // On Windows, we need to do some special handling here
      // Windows has this funky behavior where pressing AltGr registers as RAlt+LControl down - more info here:
      // https://devblogs.microsoft.com/oldnewthing/?p=40003
      | Windows(_) =>
        let altGr =
          altGr
          || Revery.Key.Keymod.isRightAltDown(keymod)
          && Revery.Key.Keymod.isControlDown(keymod);
        // If altGr is active, disregard control / alt key
        let ctrlKey = altGr ? false : control;
        let altKey = altGr ? false : alt;
        (altGr, ctrlKey, altKey);

      | Mac(_) =>
        let altGr = altGr || alt;
        let altKey = altGr ? false : alt;
        (altGr, control, altKey);

      | _ => (altGr, control, alt)
      };

    let modifiers = EditorInput.Modifiers.{shift, control, alt, super, altGr};

    keycode
    |> Internal.tryToGetSpecialKey
    |> Option.map(key =>
         [EditorInput.KeyPress.physicalKey(~key, ~modifiers)]
       )
    |> OptionEx.or_lazy(() => {
         Internal.tryToGetCharacterFromKey(
           ~shift,
           ~control,
           ~alt,
           ~super,
           ~altGr,
           ~scancode,
         )
       })
    |> Option.map(keys => EditorInput.KeyCandidate.ofList(keys));
  };
};
