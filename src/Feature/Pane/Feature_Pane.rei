/*
 * Feature_Pane.rei
 *
 * Feature for the bottom pane, hosting notifications, diagnostics, etc.
 */
open Oni_Core;

[@deriving show({with_path: false})]
type pane =
  | Search
  | Diagnostics
  | Notifications;

[@deriving show({with_path: false})]
type msg;

type outmsg =
  | Nothing
  | PopFocus(pane);

module Msg: {
  let resizeHandleDragged: int => msg;
  let resizeCommitted: msg;
};

type model;

let update: (msg, model) => (model, outmsg);

module Contributions: {
  let commands: list(Command.t(msg));
  let keybindings: list(Oni_Input.Keybindings.keybinding);
};

let initial: model;

let height: model => int;
let selected: model => pane;
let isVisible: (pane, model) => bool;
let isOpen: model => bool;

let show: (~pane: pane, model) => model;
let close: model => model;
