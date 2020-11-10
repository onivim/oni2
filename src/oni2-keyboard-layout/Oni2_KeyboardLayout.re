type callback = (~language: string, ~layout: string) => unit;

type callbackPriv = (string, string) => unit;

let wrapCallback = (callback, language, layout) =>
  callback(~language, ~layout);

let callbackListRef: ref(list(callbackPriv)) = ref([]);

let subscribe = (callback: callback) =>
  callbackListRef := [wrapCallback(callback), ...callbackListRef^];

external init: unit => unit = "oni2_KeyboardLayoutInit";

external getCurrentLayout: unit => string =
  "oni2_KeyboardLayoutGetCurrentLayout";
external getCurrentLanguage: unit => string =
  "oni2_KeyboardLayoutGetCurrentLanguage";

Callback.register("oni2_KeyboardLayoutCallbackListRef", callbackListRef);

module Keymap = {
  type entry = {
    unmodified: string,
    withShift: string,
    withAltGraph: string,
    withAltGraphShift: string,
  };

  type t = Hashtbl.t(string, entry);

  external populateCurrentKeymap: ('keymap, ('keymap, string, entry) => unit) => unit =
    "oni2_KeyboardLayoutPopulateCurrentKeymap";

  let getCurrent = () => {
    // This number was gotten by counting the number of keymaps in keycode_converter_data.h
    let keymap: t = Hashtbl.create(231);
    populateCurrentKeymap(keymap, Hashtbl.replace);
    keymap;
  };

  let entryOfKey = (keymap, key) => Hashtbl.find_opt(keymap, key);

  let size = Hashtbl.length;
};
