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
