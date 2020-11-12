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
  | FocusDown
  | PreviousTab
  | NextTab;

// UPDATE

let update: (msg, model) => (model, outmsg);

// CONTRIBUTIONS

module Contributions: {
  let commands: list(Command.t(msg));
  let contextKeys: model => WhenExpr.ContextKeys.t;
  let keybindings: list(Feature_Input.Schema.keybinding);
};
