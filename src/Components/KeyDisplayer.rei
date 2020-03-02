/*
 * KeyDisplayer.rei
 *
 * State for displaying key-presses in the UI
 */

open Oni_Core;
open Revery.UI;

type t;

let initial: t;

let add: (~time: float, string, t) => t;

let make:
  (
    ~key: React.Key.t=?,
    ~model: t,
    ~uiFont: UiFont.t,
    ~top: int=?,
    ~left: int=?,
    ~right: int=?,
    ~bottom: int=?,
    unit
  ) =>
  React.element(React.node);
