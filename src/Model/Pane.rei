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
let isVisible: t => bool;
let isTypeVisible: (paneType, t) => bool;

let setClosed: t => t;
let setOpen: paneType => t;
