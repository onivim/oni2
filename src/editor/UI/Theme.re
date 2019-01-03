/*
 * Theme.re
 *
 * Theming color / info
 */

open Revery.Core;
open Revery.UI;

type t = {
  background: Color.t,
  foreground: Color.t,
};

let default: t = {
  background: Color.hex("#2e3440"),
  foreground: Color.hex("#eceff4"),
};

let context = createContext(default);
let provider = getProvider(context);
