type callback = unit => unit;

let callbackListRef: ref(list(callback)) = ref([]);

let subscribe = (callback: callback) => {
  callbackListRef := [callback, ...callbackListRef^];

  () =>
    callbackListRef := List.filter(cb => cb !== callback, callbackListRef^);
};

let callKeyboardCallbacks = () => callbackListRef^ |> List.iter(cb => cb());

external init: unit => unit = "oni2_KeyboardLayoutInit";

external getCurrentLayout: unit => string =
  "oni2_KeyboardLayoutGetCurrentLayout";
external getCurrentLanguage: unit => string =
  "oni2_KeyboardLayoutGetCurrentLanguage";

Callback.register("oni2_CallKeyboardCallbacks", callKeyboardCallbacks);

module Keymap = {
  [@deriving show]
  type entry = {
    unmodified: option(string),
    withShift: option(string),
    withAltGraph: option(string),
    withAltGraphShift: option(string),
  };

  let entryToString =
      ({unmodified, withShift, withAltGraph, withAltGraphShift}) => {
    let optStr =
      fun
      | None => "(none)"
      | Some(str) => str;

    Printf.sprintf(
      "Unmodified: %s\n WithShift: %s\n WithAltGr: %s\n WithAltGrShift: %s\n",
      optStr(unmodified),
      optStr(withShift),
      optStr(withAltGraph),
      optStr(withAltGraphShift),
    );
  };

  type t = Hashtbl.t(Sdl2.Scancode.t, entry);

  external populateCurrentKeymap:
    ('keymap, ('keymap, Sdl2.Scancode.t, entry) => unit) => unit =
    "oni2_KeyboardLayoutPopulateCurrentKeymap";

  let getCurrent = () => {
    // This number was gotten by counting the number of keymaps in keycode_converter_data.h
    let keymap: t = Hashtbl.create(231);
    populateCurrentKeymap(keymap, Hashtbl.replace);
    keymap;
  };

  let entryOfScancode = (keymap, key) => Hashtbl.find_opt(keymap, key);

  let size = Hashtbl.length;
};
