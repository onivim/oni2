/*
 * Pane.rei
 *
 * State tracking the bottom 'UI' pane
 */

[@deriving show({with_path: false})]
type pane =
  | Search
  | Diagnostics
  | Notifications;

type t = {
  selected: pane,
  isOpen: bool,
};

let initial: t;

let isVisible: (pane, t) => bool;
