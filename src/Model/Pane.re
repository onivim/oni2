/*
 * Pane.re
 *
 * State tracking the bottom 'UI' pane
 */

[@deriving show({with_path: false})]
type paneType =
  | Search
  | Diagnostics;

type t = {
  activePane: paneType,
  isOpen: bool,
};

let initial = {activePane: Search, isOpen: false};

let getType = pane => !pane.isOpen ? None : Some(pane.activePane);

let isOpen = pane => pane.isOpen;

let isTypeOpen = (paneType, pane) =>
  pane.isOpen && pane.activePane == paneType;

let hide = pane => {...pane, isOpen: false};

let show = paneType => {activePane: paneType, isOpen: true};
