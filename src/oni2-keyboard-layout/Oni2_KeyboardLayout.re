type callback = (~language: string, ~layout: string) => unit;

type callbackPriv = (string, string) => unit;

let wrapCallback = (callback, language, layout) =>
  callback(~language, ~layout);

let callbackListRef: ref(list(callbackPriv)) = ref([]);
Callback.register("oni2_KeyboardLayoutCallbackListRef", callbackListRef);

let subscribe = (callback: callback) =>
  callbackListRef := [wrapCallback(callback), ...callbackListRef^];

external init: unit => unit = "oni2_KeyboardLayoutInit";
