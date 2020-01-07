/*
 * Pane.re
 *
 * State tracking the bottom 'UI' pane
 */

[@deriving show({with_path: false})]
type paneType =
  | Search
  | Diagnostics
  | References;

type t = {
  activePane: paneType,
  isOpen: bool,
};

let initial = {activePane: Search, isOpen: false};

let getType = pane => !pane.isOpen ? None : Some(pane.activePane);

let isOpen = pane => pane.isOpen;

let isTypeOpen = (paneType, pane) =>
  pane.isOpen && pane.activePane == paneType;

let setClosed = pane => {...pane, isOpen: false};

let setOpen = paneType => {activePane: paneType, isOpen: true};
