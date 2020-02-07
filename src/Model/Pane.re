/*
 * Pane.re
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

let initial = {selected: Search, isOpen: false};

let isVisible = (pane, model) => model.isOpen && model.selected == pane;
