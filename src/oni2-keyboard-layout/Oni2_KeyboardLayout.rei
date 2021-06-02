type callback = unit => unit;

let init: unit => unit;
let subscribe: (callback, unit) => unit;
let getCurrentLanguage: unit => string;
let getCurrentLayout: unit => string;

module Keymap: {
  [@deriving show]
  type entry = {
    unmodified: option(string),
    withShift: option(string),
    withAltGraph: option(string),
    withAltGraphShift: option(string),
  };

  type t;

  let getCurrent: unit => t;

  let entryOfScancode: (t, Sdl2.Scancode.t) => option(entry);

  let entryToString: entry => string;

  let size: t => int;
};
