type callback = unit => unit;

let init: unit => unit;
let subscribe: (callback, unit) => unit;
let getCurrentLanguage: unit => string;
let getCurrentLayout: unit => string;

module Keymap: {
  type entry = {
    unmodified: string,
    withShift: string,
    withAltGraph: string,
    withAltGraphShift: string,
  };

  type t;

  let getCurrent: unit => t;

  let entryOfKey: (t, string) => option(entry);

  let size: t => int;
};
