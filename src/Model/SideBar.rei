open Revery;
open Oni_Core;

type sideBarType =
  | FileExplorer
  | Extensions;

type t;

let initial: t;
let setOpen: sideBarType => t;
let setClosed: t => t;
let getType: t => sideBarType;
let isOpen: t => bool;
let toggle: (sideBarType, t) => t;
