/*
 * Pane.rei
 *
 * State tracking the bottom 'UI' pane
 */

[@deriving show({with_path: false})]
type paneType =
  | Search
  | Diagnostics;

type t;

let initial: t;

let getType: t => option(paneType);
let isOpen: t => bool;
let isTypeOpen: (paneType, t) => bool;

let hide: t => t;
let show: paneType => t;
