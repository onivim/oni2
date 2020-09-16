/*
 * Component_VimWindows
 *
 * A component that features can use to enable vim-style window navigation.
 */

open Oni_Core;

// MODEL

[@deriving show]
type msg;

[@deriving show]
type model;

let initial: model;

type outmsg =
  | Nothing
  | FocusLeft
  | FocusRight
  | FocusUp
  | FocusDown;

// UPDATE

let update: (msg, model) => (model, outmsg);

// CONTRIBUTIONS

module Contributions: {
  let commands: list(Command.t(msg));
  let contextKeys: list(WhenExpr.ContextKeys.Schema.entry(model));
  let keybindings: list(Oni_Input.Keybindings.keybinding);
};
