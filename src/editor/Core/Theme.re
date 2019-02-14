/*
 * Theme.re
 *
 * Theming color / info
 */

open Revery.Core;

type t = {
  background: Color.t,
  foreground: Color.t,
  editorBackground: Color.t,
  editorForeground: Color.t,
  editorLineNumberBackground: Color.t,
  editorLineNumberForeground: Color.t,
  editorActiveLineNumberForeground: Color.t,
  scrollbarSliderBackground: Color.t,
  editorMenuBackground: Color.t,
  editorMenuForeground: Color.t,
  editorMenuItemSelected: Color.t,
};

let default: t = {
  background: Color.hex("#212733"),
  foreground: Color.hex("#ECEFF4"),
  editorBackground: Color.hex("#2F3440"),
  editorForeground: Color.hex("#DCDCDC"),
  editorLineNumberBackground: Color.hex("#2F3440"),
  editorLineNumberForeground: Color.hex("#495162"),
  editorActiveLineNumberForeground: Color.hex("#737984"),
  scrollbarSliderBackground: Color.rgba(0., 0., 0., 0.2),
  editorMenuBackground: Color.hex("#2F3440"),
  editorMenuForeground: Color.hex("#FFFFFF"),
  editorMenuItemSelected: Color.hex("#495162"),
};

let create: unit => t =
  () => {
    default;
  };
