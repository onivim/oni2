open Revery;
open Oni_Core;

type sideBarType =
  | FileExplorer
  | Extensions;

type t = {
  isOpen: bool,
  activeType: sideBarType,
};

let initial = {isOpen: true, activeType: FileExplorer};

let setOpen = sideBarType => {isOpen: true, activeType: sideBarType};

let setClosed = (sideBar: t) => {...sideBar, isOpen: false};

let toggle = (sideBarType, state: t) =>
  if (sideBarType == state.activeType) {
    {...state, isOpen: !state.isOpen};
  } else {
    {isOpen: true, activeType: sideBarType};
  };
