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
  editorBackground: Color.t,
  editorForeground: Color.t,
};

let default: t = {
  background: Color.hex("#212733"),
  foreground: Color.hex("#ECEFF4"),
  editorBackground: Color.hex("#2F3440"),
  editorForeground: Color.hex("#DCDCDC"),
};

let get: unit => t = () => {
    default
}
